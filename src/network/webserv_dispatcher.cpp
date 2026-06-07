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

void WebServer::handleClientRequest(int clientFd) {
    std::map<int, ClientConnection*>::iterator it = _clientConnections.find(clientFd);
    if (it == _clientConnections.end())
        return;

    ClientConnection* conn = it->second;
    if (!conn)
        return;

    char buffer[4096];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        closeClientConnection(clientFd);
        return;
    }

    conn->request_buffer.append(buffer, static_cast<size_t>(bytesRead));
    conn->_last_active = time(NULL);

    if (!conn->_http_request)
        conn->_http_request = new HttpRequest();
    if (!conn->_http_response)
        conn->_http_response = new HttpResponse();

    if (!conn->_http_request->isRequestComplete(conn->request_buffer))
        return;

    conn->_request_complete = true;
    if (!parseHttpRequest(conn)) {
        conn->response_buffer = conn->_http_response->buildErrorResponse(400, "Bad Request", *conn->_http_request);
        conn->_response_ready = true;
        return;
    }

    std::string host = conn->_http_request->getHost();
    conn->_runtime_server = hostRouting(host, conn->_listen_port);
    if (conn->_runtime_server)
        conn->_matched_location = conn->_runtime_server->locationRouting(conn->_http_request->getURI());

    buildHttpResponse(conn);
    conn->_response_ready = true;
}

void WebServer::handleClientResponse(int clientFd) {
    std::map<int, ClientConnection*>::iterator it = _clientConnections.find(clientFd);
    if (it == _clientConnections.end())
        return;

    ClientConnection* conn = it->second;
    if (!conn || !conn->_response_ready)
        return;

    if (conn->_bytes_sent >= conn->response_buffer.size()) {
        closeClientConnection(clientFd);
        return;
    }

    ssize_t bytesSent = send(clientFd, conn->response_buffer.data() + conn->_bytes_sent,
        conn->response_buffer.size() - conn->_bytes_sent, 0);
    if (bytesSent <= 0) {
        closeClientConnection(clientFd);
        return;
    }

    conn->_bytes_sent += static_cast<size_t>(bytesSent);
    if (conn->_bytes_sent >= conn->response_buffer.size())
        closeClientConnection(clientFd);
}
