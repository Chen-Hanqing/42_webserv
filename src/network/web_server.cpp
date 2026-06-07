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

WebServer::WebServer() : _listensocketBound(false), _running(false), _maxFd(-1) {}

WebServer::~WebServer() {
    stop();
    cleanup();
}

const Config& WebServer::getConfig() const {
    return _config;
}

size_t WebServer::getServerCount() const {
    return _servers.size();
}

std::string WebServer::getLastError() const {
    return "Check console output for detailed error messages";
}

bool WebServer::validateConfig() {
    if (_config.empty()) {
        std::cerr << "No server configurations found" << std::endl;
        return false;
    }
    for (size_t i = 0; i < _config.getServerCount(); ++i) {
        const ServerBlockConfig& server = _config.getServer(i);
        if (server.listen.empty()) {
            std::cerr << "Server " << i << " has no listen ports" << std::endl;
            return false;
        }
        for (size_t j = 0; j < server.listen.size(); ++j) {
            int port = server.listen[j];
            if (port < 1 || port > 65535) {
                std::cerr << "Invalid port number: " << port << std::endl;
                return false;
            }
            _listenPorts.insert(port); // Store the port for later socket creation
        }
    }
    return true;
}

bool WebServer::createListenSockets() {
    // Create listening socket and bind the socket for each port
    for (std::set<int>::const_iterator it = _listenPorts.begin(); it != _listenPorts.end(); ++it) {
        int port = *it;
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            std::cerr << "Failed to create socket for port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            return false;
        }
        // Set SO_REUSEADDR to allow quick reuse of the port
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            std::cerr << "Failed to set SO_REUSEADDR for port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }
        // Set the socket to non-blocking mode
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            std::cerr << "Failed to set non-blocking mode for port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }
        // Bind the socket to the specified IP and port
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(static_cast<unsigned short>(port));
        if (bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            std::cerr << "Failed to bind to port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }
        // Store the listening fd and port mapping
        _listensocketFds.push_back(sockfd);
        _port2socket[port] = sockfd;
        _fd2port[sockfd] = port;
    }
    return true;
}

bool WebServer::startListening() {
    // Start listening on all bound sockets
    for (size_t i = 0; i < _listensocketFds.size(); ++i) {
        if (listen(_listensocketFds[i], SOMAXCONN) == -1) {
            std::cerr << "Failed to listen on fd " << _listensocketFds[i] << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
    return true;
}



bool WebServer::listensocket_bind(const std::string& configFile) {
    ConfigParser parser;
    if (!parser.parseFile(configFile, _config)) {
        std::cerr << "Failed to parse config file: " << parser.getLastError() << std::endl;
        return false;
    }
    return listensocket_bindFromConfig(_config);
}



bool WebServer::createRuntimeServer() {
    for (size_t i = 0; i < _config.getServerCount(); ++i) {
        const ServerBlockConfig& serverBlockConfig = _config.getServer(i);
        RuntimeServer* server = new RuntimeServer(serverBlockConfig);
        _servers.push_back(server);
    }
    return true;
}

bool WebServer::setupPortMapping() {
    for (size_t i = 0; i < _servers.size(); ++i) {
        RuntimeServer* server = _servers[i];
        const std::vector<int>& listenPorts = server->getConfig().listen;
        //set up port to server mapping for all listen ports of the server
        for (size_t j = 0; j < listenPorts.size(); ++j)
            _port2servers[listenPorts[j]].push_back(server);
    }
    return true;
}

bool WebServer::listensocket_bindFromConfig(const Config& cfg) {
    _config = cfg;
    if (!validateConfig())
        return false;
    if (!createListenSockets()) {
        cleanup();
        return false;
    }
    if (!createRuntimeServer()) {
        cleanup();
        return false;
    }
    if (!setupPortMapping()) {
        cleanup();
        return false;
    }
    _listensocketBound = true;
    return true;
}

bool WebServer::start() {
    if (!_listensocketBound)
        return false;
    if (_running)
        return true;
    if (!startListening()) {
        cleanup();
        return false;
    }
    _running = true;
    updateMaxFd();
    return true;
}

void WebServer::stop() {
    if (!_running)
        return;
    _running = false;
}

void WebServer::cleanup() {
    for (std::map<int, ClientConnection*>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); ++it) {
        if (it->first != -1)
            close(it->first);
        delete it->second;
    }
    _clientConnections.clear();

    for (size_t i = 0; i < _servers.size(); ++i)
        delete _servers[i];
    _servers.clear();

    _port2servers.clear();
    _maxFd = -1;
    _listensocketBound = false;
    for (size_t i = 0; i < _listensocketFds.size(); ++i) {
        if (_listensocketFds[i] != -1)
            close(_listensocketFds[i]);
    }
    _listensocketFds.clear();
    _port2socket.clear();
    _fd2port.clear();
    _listenPorts.clear();
}



RuntimeServer* WebServer::hostRouting(const std::string& hostHeader, int port) {
    std::map<int, std::vector<RuntimeServer*> >::iterator it = _port2servers.find(port);
    if (it == _port2servers.end())
        return NULL;

    const std::vector<RuntimeServer*>& serverList = it->second;
    for (size_t i = 0; i < serverList.size(); ++i) {
        if (serverList[i]->matchesServerName(hostHeader))
            return serverList[i];
    }
    return serverList.empty() ? NULL : serverList[0]; 
    // Return the first server if no server name matches
}



bool WebServer::isListeningOnPort(int port) const {
    return _port2socket.find(port) != _port2socket.end();
}

int WebServer::findMappedSocket(int port) const {
    std::map<int, int>::const_iterator it = _port2socket.find(port);
    if (it == _port2socket.end())
        return -1;
    return it->second;
}

void WebServer::updateMaxFd() {
    _maxFd = -1;
    for (size_t j = 0; j < _listensocketFds.size(); ++j) {
        if (_listensocketFds[j] > _maxFd)
            _maxFd = _listensocketFds[j];
    }
    for (std::map<int, ClientConnection*>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); ++it) {
        if (it->first > _maxFd)
            _maxFd = it->first;
    }
}

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
    conn->request_buffer.clear();
    conn->response_buffer.clear();
    conn->_bytes_sent = 0;
    conn->_request_complete = false;
    conn->_response_ready = false;
    delete conn->_http_request;
    delete conn->_http_response;
    conn->_http_request = NULL;
    conn->_http_response = NULL;
    conn->_runtime_server = NULL;
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
