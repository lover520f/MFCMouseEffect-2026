#include "pch.h"
#include "HttpServer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <cstdlib>
#include <sstream>
#include <string>

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {
namespace {

std::string StatusText(int code) {
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

bool SockSendAll(int socketHandle, const char* data, int len) {
    int offset = 0;
    while (offset < len) {
        const int sent = send(socketHandle, data + offset, len - offset, 0);
        if (sent <= 0) return false;
        offset += sent;
    }
    return true;
}

} // namespace

bool HttpServer::ParseRequest(int clientSock, HttpRequest& out) {
    std::string data;
    data.reserve(4096);

    char buf[2048];
    while (data.find("\r\n\r\n") == std::string::npos) {
        const int received = recv(clientSock, buf, static_cast<int>(sizeof(buf)), 0);
        if (received <= 0) return false;

        data.append(buf, buf + received);
        if (data.size() > 65536) return false;
    }

    const size_t headerEnd = data.find("\r\n\r\n");
    const std::string header = data.substr(0, headerEnd);
    const std::string rest = data.substr(headerEnd + 4);

    std::istringstream headerStream(header);
    std::string line;
    if (!std::getline(headerStream, line)) return false;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    {
        std::istringstream requestLine(line);
        if (!(requestLine >> out.method >> out.path)) return false;
    }

    int contentLen = 0;
    while (std::getline(headerStream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;

        const size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        const std::string key = ToLowerAscii(TrimAscii(line.substr(0, colonPos)));
        const std::string value = TrimAscii(line.substr(colonPos + 1));
        out.headers[key] = value;
        if (key == "content-length") {
            contentLen = std::atoi(value.c_str());
        }
    }

    if (contentLen < 0 || contentLen > 1024 * 1024) return false;
    out.body = rest;
    while (static_cast<int>(out.body.size()) < contentLen) {
        const int received = recv(clientSock, buf, static_cast<int>(sizeof(buf)), 0);
        if (received <= 0) break;
        out.body.append(buf, buf + received);
    }
    if (static_cast<int>(out.body.size()) > contentLen) {
        out.body.resize(contentLen);
    }
    return true;
}

bool HttpServer::SendResponse(int clientSock, const HttpResponse& resp) {
    std::ostringstream headStream;
    headStream << "HTTP/1.1 " << resp.statusCode << " " << StatusText(resp.statusCode) << "\r\n";
    headStream << "Content-Type: "
               << (resp.contentType.empty() ? "text/plain; charset=utf-8" : resp.contentType)
               << "\r\n";
    headStream << "Content-Length: " << resp.body.size() << "\r\n";
    headStream << "Connection: close\r\n";
    headStream << "Cache-Control: no-store\r\n";
    for (const auto& kv : resp.extraHeaders) {
        headStream << kv.first << ": " << kv.second << "\r\n";
    }
    headStream << "\r\n";

    const std::string head = headStream.str();
    if (!SockSendAll(clientSock, head.data(), static_cast<int>(head.size()))) return false;
    return SockSendAll(clientSock, resp.body.data(), static_cast<int>(resp.body.size()));
}

} // namespace mousefx
