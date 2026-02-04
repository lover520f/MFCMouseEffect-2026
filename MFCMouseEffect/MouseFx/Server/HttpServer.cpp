#include "pch.h"
#include "HttpServer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <algorithm>
#include <chrono>
#include <cctype>
#include <sstream>

namespace mousefx {

static std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

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

static std::string StatusText(int code) {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "OK";
    }
}

static bool SockSendAll(int s, const char* data, int len) {
    int off = 0;
    while (off < len) {
        int n = send(s, data + off, len - off, 0);
        if (n <= 0) return false;
        off += n;
    }
    return true;
}

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

bool HttpServer::ParseRequest(int clientSock, HttpRequest& out) {
    std::string data;
    data.reserve(4096);

    char buf[2048];
    while (data.find("\r\n\r\n") == std::string::npos) {
        int n = recv(clientSock, buf, (int)sizeof(buf), 0);
        if (n <= 0) return false;
        data.append(buf, buf + n);
        if (data.size() > 65536) return false;
    }

    size_t headerEnd = data.find("\r\n\r\n");
    std::string header = data.substr(0, headerEnd);
    std::string rest = data.substr(headerEnd + 4);

    std::istringstream ss(header);
    std::string line;
    if (!std::getline(ss, line)) return false;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    {
        std::istringstream ls(line);
        if (!(ls >> out.method >> out.path)) return false;
    }

    int contentLen = 0;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string key = ToLowerAscii(TrimAscii(line.substr(0, pos)));
        std::string val = TrimAscii(line.substr(pos + 1));
        out.headers[key] = val;
        if (key == "content-length") {
            contentLen = atoi(val.c_str());
        }
    }

    if (contentLen < 0 || contentLen > 1024 * 1024) return false;
    out.body = rest;
    while ((int)out.body.size() < contentLen) {
        int n = recv(clientSock, buf, (int)sizeof(buf), 0);
        if (n <= 0) break;
        out.body.append(buf, buf + n);
    }
    if ((int)out.body.size() > contentLen) out.body.resize(contentLen);
    return true;
}

bool HttpServer::SendResponse(int clientSock, const HttpResponse& resp) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << resp.statusCode << " " << StatusText(resp.statusCode) << "\r\n";
    ss << "Content-Type: " << (resp.contentType.empty() ? "text/plain; charset=utf-8" : resp.contentType) << "\r\n";
    ss << "Content-Length: " << resp.body.size() << "\r\n";
    ss << "Connection: close\r\n";
    ss << "Cache-Control: no-store\r\n";
    for (const auto& kv : resp.extraHeaders) {
        ss << kv.first << ": " << kv.second << "\r\n";
    }
    ss << "\r\n";
    const std::string head = ss.str();
    if (!SockSendAll(clientSock, head.data(), (int)head.size())) return false;
    return SockSendAll(clientSock, resp.body.data(), (int)resp.body.size());
}

} // namespace mousefx
