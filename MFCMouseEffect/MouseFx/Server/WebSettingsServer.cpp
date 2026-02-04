#include "pch.h"
#include "WebSettingsServer.h"

#include <algorithm>
#include <random>
#include <sstream>

#include "MouseFx/Core/AppController.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebUiAssets.h"
#include "MouseFx/ThirdParty/json.hpp"
#include "Settings/SettingsOptions.h"

using json = nlohmann::json;

namespace mousefx {

static std::string TrimAscii(std::string s) {
    auto is_space = [](unsigned char ch) {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    };
    size_t b = 0;
    while (b < s.size() && is_space((unsigned char)s[b])) b++;
    size_t e = s.size();
    while (e > b && is_space((unsigned char)s[e - 1])) e--;
    if (b == 0 && e == s.size()) return s;
    return s.substr(b, e - b);
}

static std::string Utf16ToUtf8(const wchar_t* ws) {
    if (!ws || !*ws) return {};
    int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ws, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string out((size_t)len, '\0');
    int written = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ws, -1, out.empty() ? nullptr : &out[0], len, nullptr, nullptr);
    if (written <= 0) return {};
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

static bool IsValidUtf8(const std::string& s) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s.data());
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = p[i];
        if (c < 0x80) { i++; continue; }
        if ((c >> 5) == 0x6) {
            if (i + 1 >= s.size()) return false;
            if ((p[i + 1] & 0xC0) != 0x80) return false;
            i += 2; continue;
        }
        if ((c >> 4) == 0xE) {
            if (i + 2 >= s.size()) return false;
            if ((p[i + 1] & 0xC0) != 0x80 || (p[i + 2] & 0xC0) != 0x80) return false;
            i += 3; continue;
        }
        if ((c >> 3) == 0x1E) {
            if (i + 3 >= s.size()) return false;
            if ((p[i + 1] & 0xC0) != 0x80 || (p[i + 2] & 0xC0) != 0x80 || (p[i + 3] & 0xC0) != 0x80) return false;
            i += 4; continue;
        }
        return false;
    }
    return true;
}

static std::string EnsureUtf8(const std::string& s) {
    if (s.empty()) return s;
    if (IsValidUtf8(s)) return s;

    int wlen = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return {};
    std::wstring w((size_t)wlen, L'\0');
    int wwritten = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, w.empty() ? nullptr : &w[0], wlen);
    if (wwritten <= 0) return {};
    if (!w.empty() && w.back() == L'\0') w.pop_back();

    int ulen = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ulen <= 0) return {};
    std::string out((size_t)ulen, '\0');
    int uwritten = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.empty() ? nullptr : &out[0], ulen, nullptr, nullptr);
    if (uwritten <= 0) return {};
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

static std::string LabelByLang(const std::wstring& zh, const std::wstring& en, const std::string& lang) {
    const wchar_t* ws = (lang == "zh-CN") ? zh.c_str() : en.c_str();
    return EnsureUtf8(Utf16ToUtf8(ws));
}

static json MakeOpt(const char* value, const wchar_t* zh, const wchar_t* en, const std::string& lang) {
    json o;
    o["value"] = value ? value : "";
    std::string label = LabelByLang(zh ? std::wstring(zh) : L"", en ? std::wstring(en) : L"", lang);
    if (label.empty()) label = value ? value : "";
    o["label"] = label;
    return o;
}

WebSettingsServer::WebSettingsServer(AppController* controller) : controller_(controller) {
    RotateToken();
    http_ = std::make_unique<HttpServer>();

    std::wstring base = ExeDirW();
    if (!base.empty()) base += L"\\webui";
    assets_ = std::make_unique<WebUiAssets>(base);
}

WebSettingsServer::~WebSettingsServer() {
    Stop();
}

bool WebSettingsServer::Start() {
    if (!http_) return false;
    if (http_->IsRunning()) return true;

    bool started = http_->StartLoopback([this](const HttpRequest& req, HttpResponse& resp) {
        Touch();
        try {
            std::string path = req.path;
            size_t q = path.find('?');
            if (q != std::string::npos) path = path.substr(0, q);

            const bool isApi = (path.rfind("/api/", 0) == 0);
            if (isApi) {
                auto it = req.headers.find("x-mfcmouseeffect-token");
                std::string t = (it == req.headers.end()) ? "" : TrimAscii(it->second);
                if (!IsTokenValid(t)) {
                    resp.statusCode = 401;
                    resp.contentType = "text/plain; charset=utf-8";
                    resp.body = "unauthorized";
                    return;
                }
            }

            if (req.method == "GET" && path == "/api/schema") {
                resp.contentType = "application/json; charset=utf-8";
                resp.body = BuildSchemaJson();
                return;
            }
            if (req.method == "GET" && path == "/api/state") {
                resp.contentType = "application/json; charset=utf-8";
                resp.body = BuildStateJson();
                return;
            }
            if ((req.method == "POST" || req.method == "GET") && path == "/api/reload") {
                if (controller_) controller_->HandleCommand("{\"cmd\":\"reload_config\"}");
                resp.contentType = "application/json; charset=utf-8";
                resp.body = json({{"ok", true}}).dump();
                return;
            }
            if (req.method == "POST" && path == "/api/stop") {
                resp.contentType = "application/json; charset=utf-8";
                resp.body = json({{"ok", true}}).dump();
                StopAsync();
                return;
            }
            if (req.method == "POST" && path == "/api/reset") {
                if (controller_) controller_->HandleCommand("{\"cmd\":\"reset_config\"}");
                resp.contentType = "application/json; charset=utf-8";
                resp.body = json({{"ok", true}}).dump();
                return;
            }
        if (req.method == "POST" && path == "/api/state") {
            resp.contentType = "application/json; charset=utf-8";
            resp.body = ApplyStateJson(req.body);
            return;
        }
            if (req.method == "GET" && path == "/favicon.ico") {
                resp.statusCode = 204;
                resp.contentType = "text/plain; charset=utf-8";
                resp.body.clear();
                return;
            }

            // Static assets.
            WebUiAsset asset;
            if (assets_ && assets_->TryGet(req.path, asset)) {
                resp.statusCode = 200;
                resp.contentType = asset.contentType;
                resp.body.assign((const char*)asset.bytes.data(), (size_t)asset.bytes.size());
                return;
            }

            resp.statusCode = 404;
            resp.contentType = "text/plain; charset=utf-8";
            resp.body = "not found";
        } catch (const std::exception& e) {
            const bool isApi = (req.path.rfind("/api/", 0) == 0);
            resp.statusCode = 500;
            if (isApi) {
                resp.contentType = "application/json; charset=utf-8";
                resp.body = json({{"ok", false}, {"error", e.what()}}).dump();
            } else {
                resp.contentType = "text/plain; charset=utf-8";
                resp.body = e.what();
            }
        }
    });
    if (started) {
        Touch();
        StartMonitor();
    }
    return started;
}

void WebSettingsServer::Stop() {
    if (http_) http_->Stop();
    StopMonitor();
}

bool WebSettingsServer::IsRunning() const {
    return http_ && http_->IsRunning();
}

uint16_t WebSettingsServer::Port() const {
    return http_ ? http_->Port() : 0;
}

std::string WebSettingsServer::Url() const {
    std::ostringstream ss;
    ss << "http://127.0.0.1:" << (int)Port() << "/?token=" << TokenCopy();
    return ss.str();
}

std::string WebSettingsServer::BuildSchemaJson() const {
    std::string lang = "zh-CN";
    if (controller_) {
        const auto cfg = controller_->GetConfigSnapshot();
        lang = cfg.uiLanguage.empty() ? "zh-CN" : cfg.uiLanguage;
    }

    json out;
    out["ui_languages"] = json::array({
        {{"value","zh-CN"},{"label", LabelByLang(L"\u4e2d\u6587", L"Chinese", lang)}},
        {{"value","en-US"},{"label", LabelByLang(L"\u82f1\u6587", L"English", lang)}}
    });

    out["themes"] = json::array({
        {{"value","chromatic"},{"label", LabelByLang(L"\u70ab\u5f69", L"Chromatic", lang)}},
        {{"value","neon"},{"label", LabelByLang(L"\u9713\u8679", L"Neon", lang)}},
        {{"value","scifi"},{"label", LabelByLang(L"\u79d1\u5e7b", L"Sci-Fi", lang)}},
        {{"value","minimal"},{"label", LabelByLang(L"\u6781\u7b80", L"Minimal", lang)}},
        {{"value","game"},{"label", LabelByLang(L"\u6e38\u620f\u611f", L"Game", lang)}}
    });

    auto build = [&](const EffectOption* (*fn)(size_t&), const char* key) {
        size_t n = 0;
        const EffectOption* opts = fn(n);
        json arr = json::array();
        for (size_t i = 0; i < n; ++i) {
            arr.push_back(MakeOpt(opts[i].value, opts[i].displayZh, opts[i].displayEn, lang));
        }
        out["effects"][key] = arr;
    };

    build(mousefx::ClickMetadata, "click");
    build(mousefx::TrailMetadata, "trail");
    build(mousefx::ScrollMetadata, "scroll");
    build(mousefx::HoldMetadata, "hold");
    build(mousefx::HoverMetadata, "hover");

    return out.dump();
}

std::string WebSettingsServer::BuildStateJson() const {
    if (!controller_) {
        return json({{"error","no controller"}}).dump();
    }
    const auto cfg = controller_->GetConfigSnapshot();

    json out;
    out["ui_language"] = EnsureUtf8(cfg.uiLanguage);
    out["theme"] = EnsureUtf8(cfg.theme);
    out["active"] = {
        {"click", EnsureUtf8(cfg.active.click)},
        {"trail", EnsureUtf8(cfg.active.trail)},
        {"scroll", EnsureUtf8(cfg.active.scroll)},
        {"hold", EnsureUtf8(cfg.active.hold)},
        {"hover", EnsureUtf8(cfg.active.hover)},
    };

    // Text content: flatten to comma-separated UTF-8.
    std::string text;
    for (size_t i = 0; i < cfg.textClick.texts.size(); ++i) {
        std::string utf8 = Utf16ToUtf8(cfg.textClick.texts[i].c_str());
        if (i > 0) text += ",";
        text += utf8;
    }
    out["text_content"] = text;

    out["trail_style"] = EnsureUtf8(cfg.trailStyle);
    out["trail_profiles"] = {
        {"line", {{"duration_ms", cfg.trailProfiles.line.durationMs}, {"max_points", cfg.trailProfiles.line.maxPoints}}},
        {"streamer", {{"duration_ms", cfg.trailProfiles.streamer.durationMs}, {"max_points", cfg.trailProfiles.streamer.maxPoints}}},
        {"electric", {{"duration_ms", cfg.trailProfiles.electric.durationMs}, {"max_points", cfg.trailProfiles.electric.maxPoints}}},
        {"meteor", {{"duration_ms", cfg.trailProfiles.meteor.durationMs}, {"max_points", cfg.trailProfiles.meteor.maxPoints}}},
        {"tubes", {{"duration_ms", cfg.trailProfiles.tubes.durationMs}, {"max_points", cfg.trailProfiles.tubes.maxPoints}}},
    };

    out["trail_params"] = {
        {"streamer", {{"glow_width_scale", cfg.trailParams.streamer.glowWidthScale}, {"core_width_scale", cfg.trailParams.streamer.coreWidthScale}, {"head_power", cfg.trailParams.streamer.headPower}}},
        {"electric", {{"amplitude_scale", cfg.trailParams.electric.amplitudeScale}, {"fork_chance", cfg.trailParams.electric.forkChance}}},
        {"meteor", {{"spark_rate_scale", cfg.trailParams.meteor.sparkRateScale}, {"spark_speed_scale", cfg.trailParams.meteor.sparkSpeedScale}}},
        {"idle_fade_start_ms", cfg.trailParams.idleFade.startMs},
        {"idle_fade_end_ms", cfg.trailParams.idleFade.endMs},
    };

    return out.dump();
}

std::string WebSettingsServer::ApplyStateJson(const std::string& body) {
    if (!controller_) return json({{"ok", false}, {"error", "no controller"}}).dump();

    json j;
    try {
        j = json::parse(body);
    } catch (...) {
        return json({{"ok", false}, {"error", "invalid json"}}).dump();
    }

    json cmd;
    cmd["cmd"] = "apply_settings";
    cmd["payload"] = j;
    controller_->HandleCommand(cmd.dump());
    return json({{"ok", true}}).dump();
}

std::string WebSettingsServer::MakeToken() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 15);
    std::string s;
    s.reserve(32);
    for (int i = 0; i < 32; ++i) {
        int v = dist(rng);
        s.push_back(v < 10 ? (char)('0' + v) : (char)('a' + (v - 10)));
    }
    return s;
}

std::string WebSettingsServer::Token() const {
    return TokenCopy();
}

std::string WebSettingsServer::TokenCopy() const {
    std::lock_guard<std::mutex> lock(tokenMutex_);
    return token_;
}

bool WebSettingsServer::IsTokenValid(const std::string& token) const {
    std::lock_guard<std::mutex> lock(tokenMutex_);
    return token == token_;
}

void WebSettingsServer::RotateToken() {
    std::lock_guard<std::mutex> lock(tokenMutex_);
    token_ = MakeToken();
}

std::wstring WebSettingsServer::ExeDirW() {
    wchar_t path[MAX_PATH]{};
    DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return {};
    std::wstring s(path);
    size_t pos = s.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return {};
    return s.substr(0, pos);
}

uint64_t WebSettingsServer::NowMs() {
    return GetTickCount64();
}

void WebSettingsServer::Touch() {
    lastRequestMs_.store(NowMs());
}

void WebSettingsServer::StartMonitor() {
    if (idleTimeoutMs_ <= 0) return;
    if (monitorRunning_.load()) return;
    if (monitorThread_.joinable() && std::this_thread::get_id() != monitorThread_.get_id()) {
        monitorThread_.join();
    }
    monitorRunning_.store(true);
    monitorThread_ = std::thread([this]() {
        while (monitorRunning_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (!http_ || !http_->IsRunning()) continue;
            uint64_t last = lastRequestMs_.load();
            if (last == 0) continue;
            uint64_t now = NowMs();
            if (now > last && (now - last) > (uint64_t)idleTimeoutMs_) {
                http_->Stop();
                monitorRunning_.store(false);
                break;
            }
        }
    });
}

void WebSettingsServer::StopMonitor() {
    monitorRunning_.store(false);
    if (monitorThread_.joinable() && std::this_thread::get_id() != monitorThread_.get_id()) {
        monitorThread_.join();
    }
}

void WebSettingsServer::StopAsync() {
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        Stop();
    }).detach();
}

} // namespace mousefx
