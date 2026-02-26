#include "pch.h"

#include "Platform/posix/Shell/PosixShellExitCommand.h"

#include <string_view>

namespace mousefx::platform {
namespace {

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (ToLowerAscii(lhs[i]) != ToLowerAscii(rhs[i])) {
            return false;
        }
    }
    return true;
}

} // namespace

bool IsPosixShellExitCommandLine(std::string_view line) {
    if (EqualsIgnoreCaseAscii(line, "exit")) {
        return true;
    }
    return line.find("\"cmd\"") != std::string_view::npos &&
           line.find("\"exit\"") != std::string_view::npos;
}

} // namespace mousefx::platform
