#include "web_server.hpp"
#include "runtime_server.hpp"
#include "../config/configparser.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


bool WebServer::parseHttpRequest(ClientConnection* conn) {
    if (!conn || !conn->_http_request)
        return false;
    return conn->_http_request->parseRequest(conn->request_buffer);
}

void WebServer::buildHttpResponse(ClientConnection* conn) {
    if (!conn || !conn->_http_request || !conn->_http_response)
        return;

    if (!conn->_runtime_server) {
        conn->response_buffer = conn->_http_response->buildErrorResponse(404, "Not Found", *conn->_http_request);
        return;
    }

    std::string method = conn->_http_request->getMethodStr();
    std::string uri = conn->_http_request->getURI();
    if (uri.empty())
        uri = "/";

    LocationConfig* location = conn->_matched_location;
    if (!location)
        location = conn->_runtime_server->locationRouting(uri);

    if (method == "POST") {
        conn->response_buffer = conn->_http_response->buildHttpResponse(201, "Created", *conn->_http_request);
        return;
    }

    if (method == "DELETE") {
        std::string root = conn->_runtime_server->getConfig().root;
        if (location) {
            if (!location->alias.empty())
                root = location->alias;
            else if (!location->root.empty())
                root = location->root;
        }
        if (root.empty())
            root = ".";

        std::string filePath = root;
        if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
            filePath += "/";
        if (uri.size() > 1 && uri[0] == '/')
            filePath += uri.substr(1);
        else
            filePath += uri;

        if (std::remove(filePath.c_str()) == 0)
            conn->response_buffer = conn->_http_response->buildHttpResponse(200, "OK", *conn->_http_request);
        else
            conn->response_buffer = conn->_http_response->buildErrorResponse(404, "Not Found", *conn->_http_request);
        return;
    }

    std::string root = conn->_runtime_server->getConfig().root;
    if (location) {
        if (!location->alias.empty())
            root = location->alias;
        else if (!location->root.empty())
            root = location->root;
    }
    if (root.empty())
        root = ".";

    std::string filePath = root;
    if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
        filePath += "/";
    if (uri.size() > 1 && uri[0] == '/')
        filePath += uri.substr(1);
    else
        filePath += uri;

    conn->response_buffer = conn->_http_response->buildFileResponse(filePath, *conn->_http_request);
}