#include "web_server.hpp"
#include "runtime_server.hpp"
#include "../config/configparser.hpp"
#include <iostream>

WebServer::WebServer() : listensocket_binded(false), running(false) {}

WebServer::~WebServer() {
    stop();
    cleanup();
}

bool    WebServer::listensocket_bind(const std::string& configFile){
    std::cout << "Initializing WebServer with config file: " << configFile << std::endl;
    ConfigParser    parser;
    if (!parser.parseFile(configFile, _config)){
        std::cerr << "Failed to parse config file: " << parser.getLastError() << std::endl;
        return false;
    }
    return listensocket_bindFromConfig(_config);
}

bool    WebServer::validateConfig(){
    if (_config.empty()){
        std::cerr << "No server configurations found" << std::endl;
        return false;
    }
    for (size_t i = 0; i < _config.getServerCount(); i++){
        const ServerConfig& server = _config.getServer(i);
        if (server.listen.empty()){
            std::cerr << "Server " << i << " has no listen ports" << std::endl;
            return false;
        }
        for (size_t j = 0; j < server.listen.size(); ++j){
            int port  = server.listen[j];
            if (port < 1 || port > 65535){
                std::cerr << "Invalid port number: " << port << std::endl;
                return false;
            }
            if (port < 1024){
                std::cout << "Warning: Using privileged port " << port << " (requires root privileges)" << std::endl;
            }
        }
        if (!server.root.empty())
            std::cout << "Server " << i << " root directory: " << server.root << std::endl;
    }
    return true;
}

bool    WebServer::createRuntimeServer(){
    for (size_t i = 0; i < _config.getServerCount(); i++){
        const ServerConfig& serverConfig = _config.getServer(i);
        RuntimeServer* server = new RuntimeServer(serverConfig);
        if (!server->listensocket_bind()){
            delete server;
            return false;
        }
        _servers.push_back(server);
    }
    return true;
}

bool    WebServer::setupPortMapping(){
    for (size_t i = 0; i < _servers.size(); ++i){
        RuntimeServer* server = _servers[i];
        const std::vector<int>& listenPorts = server->getConfig().listen;
        for (size_t j = 0; j < listenPorts.size(); ++j){
            int port = listenPorts[j];
            _port2servers[port].push_back(server);
        }
    }
    for (std::map<int, std::vector<RuntimeServer*> >::const_iterator it = _port2servers.begin();
            it != _port2servers.end(); ++it){
        int port = it->first;
        const std::vector<RuntimeServer*>& serverList = it->second;
        if (serverList.size() > 1){
            std::cout << "Port " << port << "is shared by " << serverList.size() << " servers (virtual hosting)" << std::endl;
            bool    hasDefault = false;
            for (size_t i = 0; i < serverList.size(); ++i){
                RuntimeServer* server = serverList[i];
                if (server->getConfig().serverName.empty()){
                    hasDefault = true;
                    break;
                }
            }
            if (!hasDefault)
                std::cout << "Warning: No default server for port " << port << std::endl;
        }
    }
    return true;
}


bool    WebServer::listensocket_bindFromConfig(const Config& cfg){
    _config = cfg;
    if (!WebServer::validateConfig()){
        std::cerr << "Configuration validation failed" << std::endl;
        return false;
    }
    if (!createRuntimeServer()){
        std::cerr << "Failed to create runtime server" << std::endl;
        cleanup();
        return false;
    }
    if (!setupPortMapping()){
        std::cerr << "Failed to set up port mapping" << std::endl;
        cleanup();
        return false;
    }
    listensocket_binded = true;
    return true;
}

bool    WebServer::start(){
    if (!listensocket_binded){
        std::cerr << "Server not listensocket_binded" << std::endl;
        return false;
    }
    if (running){
        std::cout << "Server os already running" << std::endl;
        return true;
    }
    for (size_t i = 0; i < _servers.size(); ++i){
        RuntimeServer* server = _servers[i];
        if (!server->startListening()){
            std::cerr << "Failed to start listening on server" << std::endl;
            return false;
        }
    }
}

void    WebServer::stop(){
    if (!running) return;
    std::cout << "Stopping Webserver..." << std::endl;
    for (size_t i = 0; i < _servers.size(); ++i){
        RuntimeServer* server = _servers[i];
        server->cleanup();
    }
    running = false;
    std::cout << "Webserver stopped." << std::endl;
}



void    WebServer::cleanup(){
    for (std::map<int, ClientConnection*>::iterator it = _clientConnections.begin();
    it != _clientConnections.end(); ++it){
        close(it->first);
        delete it->second;
    }
    _clientConnections.clear();
    for (size_t i = 0; i < _servers.size(); ++i){
        RuntimeServer* server = _servers[i];
        delete server;
    }
    _servers.clear();
    _port2servers.clear();
}



RuntimeServer* WebServer::findServerByHost(const std::string& hostHeader, int port){
    std::map<int, std::vector<RuntimeServer*> >::iterator it = _port2servers.find(port);
    if (it == _port2servers.end())
        return NULL;
    const std::vector<RuntimeServer*>& serverList = it->second;
    for (size_t i = 0; i < serverList.size(); ++i){
        RuntimeServer* server = serverList[i];
        if (server->matchesServerName(hostHeader))
            return server;
    }
    return serverList.empty() ? NULL : serverList[0];
}

int WebServer::getPortFromClientSocket(int clientFd){
    struct sockaddr_in  addr;
    socklen_t   addrlen = sizeof(addr);
    if (getsockname(clientFd, (struct sockaddr*)&addr, &addrlen) == 0)
        return ntohs(addr.sin_port);
    return -1;
}

const Config&   WebServer::getConfig() const {
    return _config;
}

size_t  WebServer::getServerCount() const{
    return _servers.size();
}

std::string WebServer::getLastError() const {
    return "Check console output for detailed error messages";
}

void    WebServer::handleNewConnection(int serverFd){
    while (true) {
        struct sockaddr_in  clientAddr;
        socklen_t   clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            break;
        }
        int flags = fcntl(clientFd, F_GETFL, 0);
        if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1){
            std::cerr << "Failed to set non-blocking mode" << std::endl;
            close(clientFd);
            continue;
        }
        ClientConnection*   conn = new ClientConnection(clientFd);
        conn->_last_active = time(NULL);
        _clientConnections[cliendFd] = conn;
        if (clientFd > maxFd)
            maxFd = cliendFd;
        std::cout << "New connection accepted: fd=" << clientFd << std::endl;
    }
}

bool    WebServer::parseHttpRequest(ClientConnection* conn){

}


void    WebServer::buildHttpResponse(ClientConnection* conn){
if (val_status == VALID_REQUEST){
    std::string method = conn->_http_request->getMethodStr();
    std::string uri = ;

    if (method == "GET")
        handleGetResponse(conn, uri, cgiHandler);
    else if (method == "POST")
        handlePostResponse(conn, uri, cgiHandler);
    else if (method == "DELETE")
        handleDeleteResponse(conn, uri, cgiHandler);
    else
        conn->response_buffer = conn->_http_request->buildErrorResponse();
}
}



void    WebServer::resetConnection(ClientConnection* conn){
    conn->request_buffer.clear();
    conn->response_buffer.clear();
    conn->_bytes_sent = 0;
    conn->_request_complete = false;
    conn->_response_ready = false;
    if (conn->_http_request){
        delete conn->_http_request;
        conn->_http_request = NULL;
    }
    if (conn->_http_response){
        delete conn->_http_response;
        conn->_http_response = NULL;
    }
    conn->_runtime_server = NULL;
    conn->_matched_location = NULL;
    conn->_last_active = time(NULL);
    std::cout << "Connection reset for reuse: fd=" << conn->fd << std::endl;
}

void    WebServer::closeClientConnection(int clientFd){
    std::map<int, ClientConnection*>::iterator it = clientConnections.find(clientFd);
    if (it != clientConnections.end()){
        delete it->second;
        clientConnections.erase(it);
    }
    close(clientFd);
    updateMaxFd();
    std::cout << "Connection closed: fd=" << clientFd << std::endl;
}

void    WebServer::updateMaxFd(){
    maxFd = -1;
    for (size_t i = 0; i < _servers.size(); i++){
        const std::vector<int>& socketFds = _servers[i]->getSocketFds();
        for (size_t j = 0; j < socketFds.size(); j++){
            if (socketFds[j] > maxFd)
                maxFd = socketFds[j];
        }
    }
    for (std::map<int, ClientConnection*>::iterator it = _clientConnections.begin();
    it != _clientConnections.end(); ++it){
        if (it->first > maxFd)
            maxFd = it->first;
    }
}