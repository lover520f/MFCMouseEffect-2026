#include "pch.h"

#include "MouseFx/Core/Shell/WebSettingsLaunchCoordinator.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/core/WebSettingsServer.h"

namespace mousefx {

WebSettingsLaunchCoordinator::WebSettingsLaunchCoordinator(AppController* controller)
    : controller_(controller) {
}

void WebSettingsLaunchCoordinator::ResetController(AppController* controller) {
    if (controller_ == controller) {
        return;
    }
    Stop();
    controller_ = controller;
}

void WebSettingsLaunchCoordinator::Stop() {
    if (webSettings_) {
        webSettings_->Stop();
        webSettings_.reset();
    }
}

bool WebSettingsLaunchCoordinator::IsRunning() const {
    return webSettings_ && webSettings_->IsRunning();
}

WebSettingsServer* WebSettingsLaunchCoordinator::Server() noexcept {
    return webSettings_.get();
}

const WebSettingsServer* WebSettingsLaunchCoordinator::Server() const noexcept {
    return webSettings_.get();
}

WebSettingsLaunchResult WebSettingsLaunchCoordinator::EnsureStarted() {
    WebSettingsLaunchResult result{};
    if (!controller_) {
        result.errorCode = "app_controller_missing";
        return result;
    }

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(controller_);
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            result.errorCode = "websettings_start_failed";
            result.startErrorStage = webSettings_->LastStartErrorStage();
            result.startErrorCode = webSettings_->LastStartErrorCode();
            return result;
        }
    }

    result.ok = true;
    result.url = webSettings_->Url();
    return result;
}

} // namespace mousefx
