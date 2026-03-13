#include "pch.h"
#include "HttpServer.h"

namespace mousefx {

bool HttpServer::HandleClient(SocketHandle clientSock) {
    HttpRequest request;
    if (!ParseRequest(clientSock, request)) {
        HttpResponse badRequestResponse;
        badRequestResponse.statusCode = 400;
        badRequestResponse.body = "bad request";
        return SendResponse(clientSock, badRequestResponse);
    }

    HttpResponse response;
    try {
        if (handler_) {
            handler_(request, response);
        }
    } catch (const std::exception& exception) {
        response.statusCode = 500;
        response.contentType = "text/plain; charset=utf-8";
        response.body = exception.what();
    } catch (...) {
        response.statusCode = 500;
        response.contentType = "text/plain; charset=utf-8";
        response.body = "internal error";
    }

    return SendResponse(clientSock, response);
}

} // namespace mousefx
