#pragma once

#include <string>
#include <vector>

#include "MouseFx/Core/Automation/ShortcutCaptureSession.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "Platform/PlatformApplicationCatalog.h"

namespace mousefx {

struct HttpResponse;

namespace websettings_automation_routes {

void SetJsonResponse(HttpResponse& resp, const std::string& body);
nlohmann::json ParseObjectOrEmpty(const std::string& body);
std::string ParseSessionId(const nlohmann::json& payload);
std::string PollStateToText(ShortcutCaptureSession::PollState state);
bool ParseForceRefresh(const nlohmann::json& payload);
std::vector<platform::ApplicationCatalogEntry> LoadAutomationAppCatalog(bool forceRefresh);

} // namespace websettings_automation_routes
} // namespace mousefx
