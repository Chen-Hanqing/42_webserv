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

        for (size_t j = 0; j < _listensocketFds.size(); ++j)
            FD_SET(_listensocketFds[j], &_readFds); // Add listening sockets to read set
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
        // activity < 0 means an error occurred, activity == 0 means timeout, activity > 0 means some fds are ready
        if (activity < 0) {
            if (errno == EINTR)
                continue;
            std::cerr << "select() failed: " << strerror(errno) << std::endl;
            break;
        }

        for (size_t j = 0; j < _listensocketFds.size(); ++j) {
            int serverFd = _listensocketFds[j];
            if (FD_ISSET(serverFd, &_readFds)) // check if this server's listening socket is ready
                handleNewConnection(serverFd); 
                //key function is accept() and get clientFd, then create ClientConnection
        }

        std::vector<int> clientFds;
        for (std::map<int, ClientConnection*>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); ++it)
            clientFds.push_back(it->first);

        for (size_t i = 0; i < clientFds.size(); ++i) {
            int clientFd = clientFds[i];
            if (_clientConnections.find(clientFd) != _clientConnections.end() && FD_ISSET(clientFd, &_readFds)) 
            // check if this client socket is ready for reading
                handleClientRequest(clientFd);
            // key function is recv() and read data into request_buffer, then parseHttpRequest() to check if request is complete, then buildHttpResponse()
            if (_clientConnections.find(clientFd) != _clientConnections.end() && FD_ISSET(clientFd, &_writeFds))
            // check if this client socket is ready for writing
                handleClientResponse(clientFd);
            // key function is send() and write data from response_buffer, then check if all data is sent, if so closeClientConnection()
        }
    }
}
