#include "pch.h"
#include "HttpServer.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <chrono>
#include <cerrno>

namespace mousefx {
namespace {

using NativeSocket =
#if defined(_WIN32)
    SOCKET;
#else
    int;
#endif

constexpr SocketHandle kInvalidSocketHandle =
#if defined(_WIN32)
    static_cast<SocketHandle>(INVALID_SOCKET);
#else
    static_cast<SocketHandle>(-1);
#endif

static_assert(
    sizeof(SocketHandle) >= sizeof(NativeSocket),
    "SocketHandle must not truncate native socket handles");

NativeSocket ToNativeSocket(SocketHandle socketHandle) {
    return static_cast<NativeSocket>(socketHandle);
}

void CloseSocketHandle(SocketHandle socketHandle) {
    if (socketHandle == kInvalidSocketHandle) {
        return;
    }
#if defined(_WIN32)
    closesocket(ToNativeSocket(socketHandle));
#else
    close(ToNativeSocket(socketHandle));
#endif
}

int LastSocketErrorCode() {
#if defined(_WIN32)
    return WSAGetLastError();
#else
    return errno;
#endif
}

} // namespace

bool HttpServer::StartLoopback(Handler handler) {
    return StartLoopbackOnPort(0, std::move(handler));
}

bool HttpServer::StartLoopbackOnPort(uint16_t port, Handler handler) {
    if (running_.exchange(true)) {
        return true;
    }
    handler_ = std::move(handler);
    lastStartErrorStage_.store(0);
    lastStartErrorCode_.store(0);

#if defined(_WIN32)
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        lastStartErrorStage_.store(5);
        lastStartErrorCode_.store(WSAGetLastError());
        running_.store(false);
        return false;
    }
#endif

    bool ok = false;
    const int maxAttempts = (port == 0) ? 5 : 1;
    for (int attempt = 0; attempt < maxAttempts && !ok; ++attempt) {
        listenSock_ = static_cast<SocketHandle>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        if (listenSock_ == kInvalidSocketHandle) {
            lastStartErrorStage_.store(1);
            lastStartErrorCode_.store(LastSocketErrorCode());
            break;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(port);

        int yes = 1;
#if defined(_WIN32)
        setsockopt(ToNativeSocket(listenSock_), SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
#else
        setsockopt(ToNativeSocket(listenSock_), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

        if (bind(ToNativeSocket(listenSock_), (sockaddr*)&addr, sizeof(addr)) != 0) {
            lastStartErrorStage_.store(2);
            lastStartErrorCode_.store(LastSocketErrorCode());
            CloseSocketHandle(listenSock_);
            listenSock_ = kInvalidSocketHandle;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        sockaddr_in bound{};
#if defined(_WIN32)
        int blen = sizeof(bound);
#else
        socklen_t blen = static_cast<socklen_t>(sizeof(bound));
#endif
        if (getsockname(ToNativeSocket(listenSock_), (sockaddr*)&bound, &blen) == 0) {
            port_.store(ntohs(bound.sin_port));
        }

        if (listen(ToNativeSocket(listenSock_), 8) != 0) {
            lastStartErrorStage_.store(3);
            lastStartErrorCode_.store(LastSocketErrorCode());
            CloseSocketHandle(listenSock_);
            listenSock_ = kInvalidSocketHandle;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ok = true;
    }

    if (!ok) {
        if (listenSock_ != kInvalidSocketHandle) {
            CloseSocketHandle(listenSock_);
            listenSock_ = kInvalidSocketHandle;
        }
#if defined(_WIN32)
        WSACleanup();
#endif
        port_.store(0);
        running_.store(false);
        return false;
    }

    try {
        thread_ = std::thread(&HttpServer::Run, this);
    } catch (...) {
        lastStartErrorStage_.store(4);
        lastStartErrorCode_.store(0);
        if (listenSock_ != kInvalidSocketHandle) {
            CloseSocketHandle(listenSock_);
            listenSock_ = kInvalidSocketHandle;
        }
#if defined(_WIN32)
        WSACleanup();
#endif
        port_.store(0);
        running_.store(false);
        return false;
    }
    lastStartErrorStage_.store(0);
    lastStartErrorCode_.store(0);
    return true;
}

void HttpServer::Stop() {
    const bool wasRunning = running_.exchange(false);
    if (!wasRunning) {
        return;
    }

    const SocketHandle listenSock = listenSock_;
    if (listenSock != kInvalidSocketHandle) {
        CloseSocketHandle(listenSock);
    }

    if (thread_.joinable()) {
        thread_.join();
    }

    listenSock_ = kInvalidSocketHandle;

#if defined(_WIN32)
    WSACleanup();
#endif
    port_.store(0);
}

void HttpServer::Run() {
    while (running_.load()) {
        const SocketHandle listenSock = listenSock_;
        if (listenSock == kInvalidSocketHandle) {
            break;
        }

        sockaddr_in caddr{};
#if defined(_WIN32)
        int clen = sizeof(caddr);
#else
        socklen_t clen = static_cast<socklen_t>(sizeof(caddr));
#endif
        const NativeSocket accepted = accept(ToNativeSocket(listenSock), (sockaddr*)&caddr, &clen);
        if (accepted == ToNativeSocket(kInvalidSocketHandle)) {
            if (!running_.load()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }
        const SocketHandle clientSock = static_cast<SocketHandle>(accepted);
        HandleClient(clientSock);
        CloseSocketHandle(clientSock);
    }
}

} // namespace mousefx
