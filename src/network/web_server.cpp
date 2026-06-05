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

bool WebServer::listensocket_bind(const std::string& configFile) {
    ConfigParser parser;
    if (!parser.parseFile(configFile, _config)) {
        std::cerr << "Failed to parse config file: " << parser.getLastError() << std::endl;
        return false;
    }
    return listensocket_bindFromConfig(_config);
}

bool WebServer::validateConfig() {
    if (_config.empty()) {
        std::cerr << "No server configurations found" << std::endl;
        return false;
    }
    for (size_t i = 0; i < _config.getServerCount(); ++i) {
        const ServerConfig& server = _config.getServer(i);
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
        }
    }
    return true;
}

bool WebServer::createRuntimeServer() {
    for (size_t i = 0; i < _config.getServerCount(); ++i) {
        const ServerConfig& serverConfig = _config.getServer(i);
        RuntimeServer* server = new RuntimeServer(serverConfig);
        if (!server->listensocket_bind()) {
            delete server;
            return false;
        }
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
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (!_servers[i]->startListening())
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
    for (size_t i = 0; i < _servers.size(); ++i)
        _servers[i]->cleanup();
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
}

RuntimeServer* WebServer::findServerByHost(const std::string& hostHeader, int port) {
    std::map<int, std::vector<RuntimeServer*> >::iterator it = _port2servers.find(port);
    if (it == _port2servers.end())
        return NULL;

    const std::vector<RuntimeServer*>& serverList = it->second;
    for (size_t i = 0; i < serverList.size(); ++i) {
        if (serverList[i]->matchesServerName(hostHeader))
            return serverList[i];
    }
    return serverList.empty() ? NULL : serverList[0];
}

int WebServer::getPortFromClientSocket(int clientFd) {
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    if (getsockname(clientFd, reinterpret_cast<sockaddr*>(&addr), &addrLen) == 0)
        return ntohs(addr.sin_port);
    return -1;
}

void WebServer::updateMaxFd() {
    _maxFd = -1;
    for (size_t i = 0; i < _servers.size(); ++i) {
        const std::vector<int>& socketFds = _servers[i]->getSocketFds();
        for (size_t j = 0; j < socketFds.size(); ++j) {
            if (socketFds[j] > _maxFd)
                _maxFd = socketFds[j];
        }
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
        if (clientFd > _maxFd)
            _maxFd = clientFd;
    }
}

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
        location = conn->_runtime_server->findMatchingLocation(uri);

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
