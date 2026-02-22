#include "Platform/linux/Shell/LinuxUserNotificationService.h"

#include <cstdio>
#include <cstdlib>
#include <string>

namespace mousefx {

namespace {

std::string ShellSingleQuote(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);
    out.push_back('\'');
    for (char c : value) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

} // namespace

void LinuxUserNotificationService::ShowWarning(
    const std::string& titleUtf8,
    const std::string& messageUtf8) {
    const std::string title = titleUtf8.empty() ? "MFCMouseEffect" : titleUtf8;
    const std::string message = messageUtf8.empty() ? "(empty)" : messageUtf8;

    const std::string command =
        "notify-send --urgency=normal " +
        ShellSingleQuote(title) + " " +
        ShellSingleQuote(message) + " >/dev/null 2>&1";

    if (std::system(command.c_str()) == 0) {
        return;
    }

    std::fprintf(stderr, "[mousefx][warn] %s: %s\n", title.c_str(), message.c_str());
}

} // namespace mousefx
