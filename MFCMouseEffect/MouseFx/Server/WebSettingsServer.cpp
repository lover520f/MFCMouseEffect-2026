#include "pch.h"
#include "WebSettingsServer.h"

#include <random>
#include <sstream>

#include "MouseFx/Core/AppController.h"
#include "MouseFx/Core/GpuProbeHelper.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/SettingsSchemaBuilder.h"
#include "MouseFx/Server/SettingsStateMapper.h"
#include "MouseFx/Server/WebUiAssets.h"
#include "MouseFx/ThirdParty/json.hpp"
#include "MouseFx/Utils/StringUtils.h"
#include "MouseFx/Utils/TimeUtils.h"

using json = nlohmann::json;

namespace mousefx {

WebSettingsServer::WebSettingsServer(AppController* controller) : controller_(controller) {
    RotateToken();
    http_ = std::make_unique<HttpServer>();

    std::wstring base = GetExeDirW();
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
                resp.body = controller_ ? BuildSettingsSchemaJson(controller_->GetConfigSnapshot()) : "{}";
                return;
            }
            if (req.method == "GET" && path == "/api/state") {
                resp.contentType = "application/json; charset=utf-8";
                resp.body = controller_ ? BuildSettingsStateJson(controller_->GetConfigSnapshot()) : "{}";
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
            resp.body = ApplySettingsStateJson(controller_, req.body);
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
