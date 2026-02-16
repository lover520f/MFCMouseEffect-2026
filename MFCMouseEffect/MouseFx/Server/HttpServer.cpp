#include "pch.h"
#include "HttpServer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <chrono>

namespace mousefx {

HttpServer::HttpServer() = default;

HttpServer::~HttpServer() {
    Stop();
}

bool HttpServer::StartLoopback(Handler handler) {
    if (running_.exchange(true)) return true;
    handler_ = std::move(handler);

    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        running_.store(false);
        return false;
    }

    bool ok = false;
    for (int attempt = 0; attempt < 5 && !ok; ++attempt) {
        listenSock_ = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSock_ < 0) break;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(0); // ephemeral

        int yes = 1;
        setsockopt(listenSock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

        if (bind(listenSock_, (sockaddr*)&addr, sizeof(addr)) != 0) {
            closesocket(listenSock_);
            listenSock_ = -1;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        sockaddr_in bound{};
        int blen = sizeof(bound);
        if (getsockname(listenSock_, (sockaddr*)&bound, &blen) == 0) {
            port_.store(ntohs(bound.sin_port));
        }

        if (listen(listenSock_, 8) != 0) {
            closesocket(listenSock_);
            listenSock_ = -1;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ok = true;
    }

    if (!ok) {
        if (listenSock_ >= 0) {
            closesocket(listenSock_);
            listenSock_ = -1;
        }
        WSACleanup();
        running_.store(false);
        return false;
    }

    thread_ = std::thread(&HttpServer::Run, this);
    return true;
}

void HttpServer::Stop() {
    bool wasRunning = running_.exchange(false);
    if (!wasRunning) return;

    if (listenSock_ >= 0) {
        closesocket(listenSock_);
        listenSock_ = -1;
    }

    if (thread_.joinable()) {
        thread_.join();
    }

    WSACleanup();
}

void HttpServer::Run() {
    while (running_.load()) {
        sockaddr_in caddr{};
        int clen = sizeof(caddr);
        SOCKET cs = accept(listenSock_, (sockaddr*)&caddr, &clen);
        if (cs == INVALID_SOCKET) {
            if (!running_.load()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }
        HandleClient((int)cs);
        closesocket(cs);
    }
}

bool HttpServer::HandleClient(int clientSock) {
    HttpRequest req;
    if (!ParseRequest(clientSock, req)) {
        HttpResponse resp;
        resp.statusCode = 400;
        resp.body = "bad request";
        return SendResponse(clientSock, resp);
    }

    HttpResponse resp;
    try {
        if (handler_) handler_(req, resp);
    } catch (const std::exception& e) {
        resp.statusCode = 500;
        resp.contentType = "text/plain; charset=utf-8";
        resp.body = e.what();
    } catch (...) {
        resp.statusCode = 500;
        resp.contentType = "text/plain; charset=utf-8";
        resp.body = "internal error";
    }
    return SendResponse(clientSock, resp);
}

} // namespace mousefx
