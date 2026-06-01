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

void WebServer::run() {
    if (!_running)
        return;

    while (_running) {
        FD_ZERO(&_readFds);
        FD_ZERO(&_writeFds);
        updateMaxFd();

        for (size_t i = 0; i < _servers.size(); ++i) {
            const std::vector<int>& socketFds = _servers[i]->getSocketFds();
            for (size_t j = 0; j < socketFds.size(); ++j)
                FD_SET(socketFds[j], &_readFds);
        }

        for (std::map<int, ClientConnection*>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); ++it) {
            FD_SET(it->first, &_readFds);
            if (it->second && it->second->_response_ready)
                FD_SET(it->first, &_writeFds);
        }

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(_maxFd + 1, &_readFds, &_writeFds, NULL, &timeout);
        if (activity < 0) {
            if (errno == EINTR)
                continue;
            std::cerr << "select() failed: " << strerror(errno) << std::endl;
            break;
        }

        for (size_t i = 0; i < _servers.size(); ++i) {
            const std::vector<int>& socketFds = _servers[i]->getSocketFds();
            for (size_t j = 0; j < socketFds.size(); ++j) {
                int serverFd = socketFds[j];
                if (FD_ISSET(serverFd, &_readFds))
                    handleNewConnection(serverFd);
            }
        }

        std::vector<int> clientFds;
        for (std::map<int, ClientConnection*>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); ++it)
            clientFds.push_back(it->first);

        for (size_t i = 0; i < clientFds.size(); ++i) {
            int clientFd = clientFds[i];
            if (_clientConnections.find(clientFd) != _clientConnections.end() && FD_ISSET(clientFd, &_readFds))
                handleClientRequest(clientFd);
            if (_clientConnections.find(clientFd) != _clientConnections.end() && FD_ISSET(clientFd, &_writeFds))
                handleClientResponse(clientFd);
        }
    }
}
