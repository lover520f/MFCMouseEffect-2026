#include "Platform/macos/Shell/MacosSingleInstanceGuard.h"

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <string>

namespace mousefx {

namespace {

std::string BuildLockPath(const std::string& normalizedKey) {
    return "/tmp/mfcmouseeffect_" + normalizedKey + ".lock";
}

} // namespace

MacosSingleInstanceGuard::~MacosSingleInstanceGuard() {
    Release();
}

bool MacosSingleInstanceGuard::Acquire(const std::wstring& key) {
    if (lockFd_ >= 0) {
        return true;
    }

    const std::string lockPath = BuildLockPath(NormalizeKey(key));
    lockFd_ = ::open(lockPath.c_str(), O_CREAT | O_RDWR, 0666);
    if (lockFd_ < 0) {
        return false;
    }
    if (::flock(lockFd_, LOCK_EX | LOCK_NB) != 0) {
        ::close(lockFd_);
        lockFd_ = -1;
        return false;
    }
    return true;
}

void MacosSingleInstanceGuard::Release() {
    if (lockFd_ < 0) {
        return;
    }
    ::flock(lockFd_, LOCK_UN);
    ::close(lockFd_);
    lockFd_ = -1;
}

std::string MacosSingleInstanceGuard::NormalizeKey(const std::wstring& key) {
    std::string out;
    out.reserve(key.size());
    for (wchar_t wc : key) {
        const char c = (wc >= 0 && wc < 128) ? static_cast<char>(wc) : '_';
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9')) {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    if (out.empty()) {
        out = "default";
    }
    return out;
}

} // namespace mousefx
