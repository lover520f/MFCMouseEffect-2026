#include "pch.h"
#include "WebSettingsServer.h"

#include <algorithm>
#include <random>
#include <sstream>

#include "MouseFx/Core/AppController.h"
#include "MouseFx/Core/OverlayHostService.h"
#include "MouseFx/Gpu/DawnOverlayBridge.h"
#include "MouseFx/Gpu/DawnRuntime.h"
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

static json BuildDawnProbeJson(const gpu::DawnRuntimeProbeInfo& probe) {
    return json{
        {"generation", probe.generation},
        {"compiled", probe.compiled},
        {"has_display_adapter", probe.hasDisplayAdapter},
        {"module_loaded", probe.moduleLoaded},
        {"module_name", probe.moduleName},
        {"has_core_proc", probe.hasCoreProc},
        {"has_create_instance", probe.hasCreateInstance},
        {"has_request_adapter", probe.hasRequestAdapter},
        {"has_request_device", probe.hasRequestDevice},
        {"can_create_instance", probe.canCreateInstance},
        {"can_request_adapter", probe.canRequestAdapter},
        {"can_create_device", probe.canCreateDevice},
        {"detail", probe.detail},
    };
}

static json BuildDawnOverlayBridgeJson(const gpu::DawnOverlayBridgeStatus& bridge) {
    const bool hostCompat = bridge.mode == "host_compat";
    const bool compositor = bridge.mode == "compositor";
    return json{
        {"compiled", bridge.compiled},
        {"available", bridge.available},
        {"mode", bridge.mode},
        {"mode_label_en", hostCompat ? "Host-Compatible Bridge" : (compositor ? "GPU Compositor Bridge" : "Not Enabled")},
        {"mode_label_zh", hostCompat ? u8"\u5bbf\u4e3b\u517c\u5bb9\u6865\u63a5" : (compositor ? u8"GPU \u5408\u6210\u6865\u63a5" : u8"\u672a\u542f\u7528")},
        {"detail", bridge.detail},
    };
}

static const char* DawnStateCodeFromDetail(const std::string& detail) {
    if (detail == "no_display_adapter") return "no_display_adapter";
    if (detail == "dawn_disabled_at_build") return "build_disabled";
    if (detail == "dawn_loader_missing") return "loader_missing";
    if (detail == "dawn_symbols_missing") return "symbols_missing";
    if (detail == "dawn_symbols_partial") return "symbols_partial";
    if (detail == "dawn_create_instance_proc_missing") return "create_proc_missing";
    if (detail == "dawn_create_instance_failed") return "create_failed";
    if (detail == "dawn_instance_ok_no_device") return "instance_ok_no_device";
    if (detail == "dawn_device_ready_cpu_bridge_pending") return "device_ready_cpu_bridge_pending";
    if (detail == "dawn_request_adapter_proc_missing") return "request_adapter_proc_missing";
    if (detail == "dawn_request_adapter_timeout") return "request_adapter_timeout";
    if (detail == "dawn_request_adapter_failed") return "request_adapter_failed";
    if (detail == "dawn_request_device_proc_missing") return "request_device_proc_missing";
    if (detail == "dawn_request_device_timeout") return "request_device_timeout";
    if (detail == "dawn_request_device_failed") return "request_device_failed";
    if (detail == "dawn_overlay_bridge_ready") return "overlay_bridge_ready";
    if (detail == "dawn_runtime_ready_for_device_stage") return "ready_for_device_stage";
    if (detail == "init_not_run") return "init_not_run";
    return "unknown";
}

static json BuildDawnStatusJson(const gpu::DawnRuntimeStatus& status) {
    const std::string statusDetail = status.lastInitDetail.empty() ? status.probe.detail : status.lastInitDetail;
    const char* stateCode = DawnStateCodeFromDetail(statusDetail);
    return json{
        {"schema_version", 1},
        {"state_code", stateCode},
        {"detail", statusDetail},
        {"init_attempts", status.initAttempts},
        {"last_init_detail", status.lastInitDetail},
        {"last_init_tick_ms", status.lastInitTickMs},
        {"ready_for_device_stage", status.readyForDeviceStage},
        {"legacy_ready_for_device_stage", status.readyForDeviceStage},
        {"probe", BuildDawnProbeJson(status.probe)},
    };
}

static json BuildDawnAdviceJson(const std::string& stateCode) {
    if (stateCode == "ready_for_device_stage" || stateCode == "instance_ok_no_device") {
        return json{
            {"action_code", "wire_device_stage"},
            {"action_text_en", "Dawn runtime is ready. Wire adapter/device/surface initialization next."},
            {"action_text_zh", u8"\u0044\u0061\u0077\u006e \u8fd0\u884c\u65f6\u5df2\u5c31\u7eea\uff0c\u4e0b\u4e00\u6b65\u8bf7\u63a5\u5165 \u0061\u0064\u0061\u0070\u0074\u0065\u0072\u002f\u0064\u0065\u0076\u0069\u0063\u0065\u002f\u0073\u0075\u0072\u0066\u0061\u0063\u0065 \u521d\u59cb\u5316\u3002"},
            {"tone", "info"},
        };
    }
    if (stateCode == "overlay_bridge_ready") {
        return json{
            {"action_code", "enable_dawn_backend"},
            {"action_text_en", "Dawn overlay bridge is ready. You can switch backend to Dawn to enable GPU mode."},
            {"action_text_zh", u8"\u0044\u0061\u0077\u006e \u6e32\u67d3\u6865\u5df2\u5c31\u7eea\u3002\u53ef\u5207\u6362\u5230 \u0044\u0061\u0077\u006e \u540e\u7aef\u542f\u7528 GPU \u6a21\u5f0f\u3002"},
            {"tone", "ok"},
        };
    }
    if (stateCode == "device_ready_cpu_bridge_pending") {
        return json{
            {"action_code", "wire_overlay_gpu_bridge"},
            {"action_text_en", "Adapter/device handshake succeeded. Next wire OverlayHost rendering bridge to Dawn backend."},
            {"action_text_zh", u8"\u5df2\u5b8c\u6210 \u0061\u0064\u0061\u0070\u0074\u0065\u0072\u002f\u0064\u0065\u0076\u0069\u0063\u0065 \u63e1\u624b\u3002\u4e0b\u4e00\u6b65\u8bf7\u5c06 \u004f\u0076\u0065\u0072\u006c\u0061\u0079\u0048\u006f\u0073\u0074 \u6e32\u67d3\u6865\u63a5\u5165 \u0044\u0061\u0077\u006e \u540e\u7aef\u3002"},
            {"tone", "info"},
        };
    }
    if (stateCode == "request_adapter_proc_missing" || stateCode == "request_device_proc_missing") {
        return json{
            {"action_code", "replace_runtime_binary"},
            {"action_text_en", "Runtime is missing requestAdapter/requestDevice symbols. Replace with a newer compatible Dawn runtime."},
            {"action_text_zh", u8"\u8fd0\u884c\u65f6\u7f3a\u5c11 \u0072\u0065\u0071\u0075\u0065\u0073\u0074\u0041\u0064\u0061\u0070\u0074\u0065\u0072\u002f\u0072\u0065\u0071\u0075\u0065\u0073\u0074\u0044\u0065\u0076\u0069\u0063\u0065 \u7b26\u53f7\u3002\u8bf7\u66ff\u6362\u4e3a\u66f4\u65b0\u4e14\u517c\u5bb9\u7684 \u0044\u0061\u0077\u006e \u8fd0\u884c\u65f6\u3002"},
            {"tone", "warn"},
        };
    }
    if (stateCode == "request_adapter_timeout" || stateCode == "request_device_timeout" ||
        stateCode == "request_adapter_failed" || stateCode == "request_device_failed") {
        return json{
            {"action_code", "check_driver_and_backend"},
            {"action_text_en", "Adapter/device request failed. Verify graphics driver, runtime build, and desktop session capability."},
            {"action_text_zh", u8"\u8bf7\u6c42 \u0061\u0064\u0061\u0070\u0074\u0065\u0072\u002f\u0064\u0065\u0076\u0069\u0063\u0065 \u5931\u8d25\u3002\u8bf7\u68c0\u67e5\u663e\u5361\u9a71\u52a8\u3001\u8fd0\u884c\u65f6\u7248\u672c\u4ee5\u53ca\u684c\u9762\u4f1a\u8bdd\u80fd\u529b\u3002"},
            {"tone", "warn"},
        };
    }
    if (stateCode == "loader_missing") {
        return json{
            {"action_code", "install_dawn_runtime"},
            {"action_text_en", "Dawn loader DLL was not found. Deploy webgpu_dawn.dll or dawn_native.dll next to the exe."},
            {"action_text_zh", u8"\u672a\u627e\u5230 \u0044\u0061\u0077\u006e \u8fd0\u884c\u65f6 \u0044\u004c\u004c\u3002\u8bf7\u5c06 \u0077\u0065\u0062\u0067\u0070\u0075\u005f\u0064\u0061\u0077\u006e\u002e\u0064\u006c\u006c \u6216 \u0064\u0061\u0077\u006e\u005f\u006e\u0061\u0074\u0069\u0076\u0065\u002e\u0064\u006c\u006c \u90e8\u7f72\u5230\u53ef\u6267\u884c\u6587\u4ef6\u540c\u76ee\u5f55\u3002"},
            {"tone", "warn"},
        };
    }
    if (stateCode == "symbols_missing" || stateCode == "symbols_partial") {
        return json{
            {"action_code", "replace_runtime_binary"},
            {"action_text_en", "Dawn DLL was found but required symbols are missing. Replace with a compatible runtime build."},
            {"action_text_zh", u8"\u5df2\u627e\u5230 \u0044\u0061\u0077\u006e \u0044\u004c\u004c\uff0c\u4f46\u7f3a\u5c11\u6240\u9700\u7b26\u53f7\u3002\u8bf7\u66ff\u6362\u4e3a\u517c\u5bb9\u7248\u672c\u7684\u8fd0\u884c\u65f6\u4e8c\u8fdb\u5236\u3002"},
            {"tone", "warn"},
        };
    }
    if (stateCode == "create_failed" || stateCode == "create_proc_missing") {
        return json{
            {"action_code", "validate_runtime_abi"},
            {"action_text_en", "CreateInstance failed. Verify runtime ABI compatibility and graphics driver environment."},
            {"action_text_zh", u8"\u0043\u0072\u0065\u0061\u0074\u0065\u0049\u006e\u0073\u0074\u0061\u006e\u0063\u0065 \u5931\u8d25\u3002\u8bf7\u68c0\u67e5\u8fd0\u884c\u65f6 \u0041\u0042\u0049 \u517c\u5bb9\u6027\u53ca\u663e\u5361\u9a71\u52a8\u73af\u5883\u3002"},
            {"tone", "warn"},
        };
    }
    if (stateCode == "build_disabled") {
        return json{
            {"action_code", "enable_dawn_build_flag"},
            {"action_text_en", "Dawn backend is disabled at build time. Enable MOUSEFX_ENABLE_DAWN and rebuild."},
            {"action_text_zh", u8"\u5f53\u524d\u6784\u5efa\u672a\u542f\u7528 \u0044\u0061\u0077\u006e\u3002\u8bf7\u5f00\u542f \u004d\u004f\u0055\u0053\u0045\u0046\u0058\u005f\u0045\u004e\u0041\u0042\u004c\u0045\u005f\u0044\u0041\u0057\u004e \u5e76\u91cd\u65b0\u7f16\u8bd1\u3002"},
            {"tone", "warn"},
        };
    }
    if (stateCode == "no_display_adapter") {
        return json{
            {"action_code", "check_display_adapter"},
            {"action_text_en", "No desktop display adapter detected. Check remote/virtual display environment."},
            {"action_text_zh", u8"\u672a\u68c0\u6d4b\u5230\u684c\u9762\u663e\u793a\u9002\u914d\u5668\u3002\u8bf7\u68c0\u67e5\u8fdc\u7a0b\u4f1a\u8bdd\u6216\u865a\u62df\u663e\u793a\u73af\u5883\u3002"},
            {"tone", "warn"},
        };
    }
    if (stateCode == "init_not_run") {
        return json{
            {"action_code", "trigger_probe_now"},
            {"action_text_en", "Initialization was not attempted yet. Trigger probe_now or apply backend once."},
            {"action_text_zh", u8"\u5c1a\u672a\u6267\u884c\u521d\u59cb\u5316\u5c1d\u8bd5\u3002\u8bf7\u8c03\u7528 \u0070\u0072\u006f\u0062\u0065\u005f\u006e\u006f\u0077 \u6216\u5207\u6362\u4e00\u6b21\u540e\u7aef\u89e6\u53d1\u63a2\u6d4b\u3002"},
            {"tone", "info"},
        };
    }
    return json{
        {"action_code", "review_logs"},
        {"action_text_en", "Unknown Dawn state. Review runtime logs and probe details."},
        {"action_text_zh", u8"\u672a\u77e5 \u0044\u0061\u0077\u006e \u72b6\u6001\uff0c\u8bf7\u7ed3\u5408\u65e5\u5fd7\u548c \u0070\u0072\u006f\u0062\u0065 \u8be6\u60c5\u6392\u67e5\u3002"},
        {"tone", "warn"},
    };
}

static json BuildGpuBannerJson(const std::string& activeBackend, const gpu::DawnRuntimeStatus& status) {
    const bool gpuInUse = (activeBackend == "dawn");
    const std::string statusDetail = status.lastInitDetail.empty() ? status.probe.detail : status.lastInitDetail;
    const std::string stateCode = DawnStateCodeFromDetail(statusDetail);
    const json advice = BuildDawnAdviceJson(stateCode);
    if (gpuInUse) {
        return json{
            {"code", "gpu_active"},
            {"tone", "ok"},
            {"text_en", "GPU backend active (Dawn)."},
            {"text_zh", u8"\u5f53\u524d\u4f7f\u7528 \u0047\u0050\u0055 \u540e\u7aef\uff08\u0044\u0061\u0077\u006e\uff09\u3002"},
            {"state_code", stateCode},
            {"action", advice},
        };
    }
    return json{
        {"code", "cpu_fallback"},
        {"tone", advice.value("tone", "warn")},
        {"text_en", "CPU fallback active."},
        {"text_zh", u8"\u5f53\u524d\u4e3a \u0043\u0050\u0055 \u515c\u5e95\u6a21\u5f0f\u3002"},
        {"state_code", stateCode},
        {"action", advice},
    };
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
            if (req.method == "POST" && path == "/api/gpu/probe_refresh") {
                OverlayHostService::Instance().RefreshGpuRuntimeProbe();
                resp.contentType = "application/json; charset=utf-8";
                resp.body = json({{"ok", true}}).dump();
                return;
            }
            if (req.method == "POST" && path == "/api/gpu/probe_now") {
                bool refresh = true;
                if (!req.body.empty()) {
                    try {
                        json in = json::parse(req.body);
                        if (in.contains("refresh") && in["refresh"].is_boolean()) {
                            refresh = in["refresh"].get<bool>();
                        }
                    } catch (...) {}
                }
                const std::string detail = OverlayHostService::Instance().ProbeDawnRuntimeNow(refresh);
                const std::string activeBackend = OverlayHostService::Instance().GetActiveRenderBackend();
                const gpu::DawnRuntimeStatus dawnStatus = gpu::GetDawnRuntimeStatus();
                const gpu::DawnOverlayBridgeStatus dawnBridge = gpu::GetDawnOverlayBridgeStatus();
                resp.contentType = "application/json; charset=utf-8";
                resp.body = json({
                    {"ok", true},
                    {"detail", detail},
                    {"active_backend", activeBackend},
                    {"backend_detail", OverlayHostService::Instance().GetRenderBackendDetail()},
                    {"gpu_in_use", activeBackend == "dawn"},
                    {"gpu_hardware_available", OverlayHostService::Instance().HasGpuHardware()},
                    {"dawn_available", OverlayHostService::Instance().IsGpuBackendAvailable("dawn")},
                    {"dawn_probe", BuildDawnProbeJson(dawnStatus.probe)},
                    {"dawn_status", BuildDawnStatusJson(dawnStatus)},
                    {"dawn_overlay_bridge", BuildDawnOverlayBridgeJson(dawnBridge)},
                    {"gpu_status_banner", BuildGpuBannerJson(activeBackend, dawnStatus)},
                }).dump();
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

    out["render_backends"] = json::array({
        {{"value","auto"},{"label", LabelByLang(L"\u81ea\u52a8\uff08\u4f18\u5148 Dawn\uff0c\u4e0d\u53ef\u7528\u5219 CPU\uff09", L"Auto (Prefer Dawn, fallback CPU)", lang)}},
        {{"value","dawn"},{"label", LabelByLang(L"Dawn GPU", L"Dawn GPU", lang)}},
        {{"value","cpu"},{"label", LabelByLang(L"CPU \u517c\u5bb9", L"CPU Fallback", lang)}}
    });

    out["hold_follow_modes"] = json::array({
        {{"value","precise"},{"label", LabelByLang(L"\u7cbe\u51c6\u8ddf\u968f\uff08\u4f4e\u5ef6\u8fdf\uff09", L"Precise (Low Latency)", lang)}},
        {{"value","smooth"},{"label", LabelByLang(L"\u5e73\u6ed1\u8ddf\u968f\uff08\u63a8\u8350\uff09", L"Smooth (Recommended)", lang)}},
        {{"value","efficient"},{"label", LabelByLang(L"\u6027\u80fd\u4f18\u5148\uff08\u7701CPU\uff09", L"Performance First (Lower CPU)", lang)}}
    });

    out["gpu_status_schema"] = {
        {"version", 1},
        {"bridge_codes", json::array({
            "bridge_not_compiled",
            "bridge_compiled_stub",
            "bridge_enabled_host_compat"
        })},
        {"bridge_modes", json::array({
            "none",
            "host_compat",
            "compositor"
        })},
        {"state_codes", json::array({
            "init_not_run",
            "no_display_adapter",
            "build_disabled",
            "loader_missing",
            "symbols_missing",
            "symbols_partial",
            "create_proc_missing",
            "create_failed",
            "instance_ok_no_device",
            "device_ready_cpu_bridge_pending",
            "request_adapter_proc_missing",
            "request_adapter_timeout",
            "request_adapter_failed",
            "request_device_proc_missing",
            "request_device_timeout",
            "request_device_failed",
            "overlay_bridge_ready",
            "ready_for_device_stage",
            "unknown"
        })},
        {"action_codes", json::array({
            "wire_device_stage",
            "wire_overlay_gpu_bridge",
            "enable_dawn_backend",
            "install_dawn_runtime",
            "replace_runtime_binary",
            "check_driver_and_backend",
            "validate_runtime_abi",
            "enable_dawn_build_flag",
            "check_display_adapter",
            "trigger_probe_now",
            "review_logs"
        })}
    };

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
    out["render_backend"] = EnsureUtf8(cfg.renderBackend);
    const std::string activeBackend = OverlayHostService::Instance().GetActiveRenderBackend();
    out["render_backend_active"] = activeBackend;
    out["render_backend_detail"] = OverlayHostService::Instance().GetRenderBackendDetail();
    out["gpu_in_use"] = (activeBackend == "dawn");
    out["gpu_hardware_available"] = OverlayHostService::Instance().HasGpuHardware();
    out["dawn_available"] = OverlayHostService::Instance().IsGpuBackendAvailable("dawn");
    const gpu::DawnRuntimeStatus dawnStatus = gpu::GetDawnRuntimeStatus();
    const gpu::DawnOverlayBridgeStatus dawnBridge = gpu::GetDawnOverlayBridgeStatus();
    out["dawn_probe"] = BuildDawnProbeJson(dawnStatus.probe);
    out["dawn_status"] = BuildDawnStatusJson(dawnStatus);
    out["dawn_overlay_bridge"] = BuildDawnOverlayBridgeJson(dawnBridge);
    out["gpu_status_banner"] = BuildGpuBannerJson(activeBackend, dawnStatus);
    out["hold_follow_mode"] = EnsureUtf8(cfg.holdFollowMode);
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
