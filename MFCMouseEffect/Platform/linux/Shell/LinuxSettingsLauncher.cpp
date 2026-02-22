#include "Platform/linux/Shell/LinuxSettingsLauncher.h"

#include <cstdlib>
#include <string>

namespace mousefx {

namespace {

bool IsUrlShellSafe(const std::string& url) {
    if (url.empty()) {
        return false;
    }
    for (char c : url) {
        if (c == '"' || c == '`' || c == '$' || c == '\n' || c == '\r') {
            return false;
        }
    }
    return true;
}

} // namespace

bool LinuxSettingsLauncher::OpenUrlUtf8(const std::string& url) {
    if (!IsUrlShellSafe(url)) {
        return false;
    }
    const std::string command = "xdg-open \"" + url + "\" >/dev/null 2>&1";
    return std::system(command.c_str()) == 0;
}

} // namespace mousefx
