#include "pch.h"
#include "HttpServer.h"

namespace mousefx {

HttpServer::HttpServer() = default;

HttpServer::~HttpServer() {
    Stop();
}

} // namespace mousefx
