#include "pch.h"
#include "HttpServer.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cstdlib>
#include <cctype>
#include <limits>
#include <sstream>
#include <string>

namespace mousefx {
namespace {

using NativeSocket =
#if defined(_WIN32)
    SOCKET;
#else
    int;
#endif

static_assert(
    sizeof(SocketHandle) >= sizeof(NativeSocket),
    "SocketHandle must not truncate native socket handles");

NativeSocket ToNativeSocket(SocketHandle socketHandle) {
    return static_cast<NativeSocket>(socketHandle);
}

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

bool SockSendAll(SocketHandle socketHandle, const char* data, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        const size_t remaining = len - offset;
        const int chunkLen = (remaining > static_cast<size_t>(std::numeric_limits<int>::max()))
            ? std::numeric_limits<int>::max()
            : static_cast<int>(remaining);
        const auto sent = send(ToNativeSocket(socketHandle), data + offset, chunkLen, 0);
        if (sent <= 0) return false;
        offset += static_cast<size_t>(sent);
    }
    return true;
}

std::string TrimAsciiCopy(const std::string& value) {
    size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
        ++begin;
    }
    size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::string ToLowerAsciiCopy(std::string value) {
    for (char& c : value) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return value;
}

} // namespace

bool HttpServer::ParseRequest(SocketHandle clientSock, HttpRequest& out) {
    std::string data;
    data.reserve(4096);

    char buf[2048];
    while (data.find("\r\n\r\n") == std::string::npos) {
        const auto received = recv(ToNativeSocket(clientSock), buf, static_cast<int>(sizeof(buf)), 0);
        if (received <= 0) return false;

        data.append(buf, buf + static_cast<size_t>(received));
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

        const std::string key = ToLowerAsciiCopy(TrimAsciiCopy(line.substr(0, colonPos)));
        const std::string value = TrimAsciiCopy(line.substr(colonPos + 1));
        out.headers[key] = value;
        if (key == "content-length") {
            contentLen = std::atoi(value.c_str());
        }
    }

    if (contentLen < 0 || contentLen > 1024 * 1024) return false;
    out.body = rest;
    while (static_cast<int>(out.body.size()) < contentLen) {
        const auto received = recv(ToNativeSocket(clientSock), buf, static_cast<int>(sizeof(buf)), 0);
        if (received <= 0) break;
        out.body.append(buf, buf + static_cast<size_t>(received));
    }
    if (static_cast<int>(out.body.size()) < contentLen) {
        return false;
    }
    if (static_cast<int>(out.body.size()) > contentLen) {
        out.body.resize(contentLen);
    }
    return true;
}

bool HttpServer::SendResponse(SocketHandle clientSock, const HttpResponse& resp) {
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
    if (!SockSendAll(clientSock, head.data(), head.size())) return false;
    return SockSendAll(clientSock, resp.body.data(), resp.body.size());
}

} // namespace mousefx
