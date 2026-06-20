#include "web_server.hpp"
#include "server_block.hpp"
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

//key function is accept() and get clientFd, then create ClientConnection
void WebServer::handleNewConnection(int serverFd) {
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        // accept() returns clientFd.
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
            break;
        }

        int flags = fcntl(clientFd, F_GETFL, 0);
        if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1) {
            close(clientFd);
            continue;
        }

        ClientConnection* conn = new ClientConnection(clientFd);
        _clientConnections[clientFd] = conn;
        conn->_listen_port = _fd2port[serverFd];
        if (clientFd > _maxFd)
            _maxFd = clientFd;
    }
}


void WebServer::resetConnection(ClientConnection* conn) {
    if (!conn)
        return;
    conn->_bytes_sent = 0;
    conn->_request_complete = false;
    conn->_response_ready = false;
    conn->_continueSent = false;
    delete conn->_http_request;
    delete conn->_http_response;
    conn->_http_request = NULL;
    conn->_http_response = NULL;
    conn->_server_block = NULL;
    conn->_matched_location = NULL;
    conn->_last_active = time(NULL);
}

void WebServer::closeClientConnection(int clientFd) {
    std::map<int, ClientConnection*>::iterator it = _clientConnections.find(clientFd);
    if (it != _clientConnections.end()) {
        delete it->second;
        _clientConnections.erase(it);
    }
    if (clientFd != -1)
        close(clientFd);
    updateMaxFd();
}
