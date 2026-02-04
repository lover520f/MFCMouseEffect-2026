#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace mousefx {

struct HttpRequest {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int statusCode = 200;
    std::string contentType = "text/plain; charset=utf-8";
    std::string body;
    std::vector<std::pair<std::string, std::string>> extraHeaders;
};

class HttpServer final {
public:
    using Handler = std::function<void(const HttpRequest&, HttpResponse&)>;

    HttpServer();
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    bool StartLoopback(Handler handler);
    void Stop();

    bool IsRunning() const { return running_.load(); }
    uint16_t Port() const { return port_.load(); }

private:
    void Run();
    bool HandleClient(int clientSock);
    bool ParseRequest(int clientSock, HttpRequest& out);
    bool SendResponse(int clientSock, const HttpResponse& resp);

    Handler handler_{};
    std::atomic<bool> running_{false};
    std::atomic<uint16_t> port_{0};
    int listenSock_ = -1;
    std::thread thread_{};
};

} // namespace mousefx

