#include "runtime_server.hpp"

#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

RuntimeServer::RuntimeServer(const ServerConfig& serverConfig) : _config(serverConfig) {}

RuntimeServer::~RuntimeServer() {
    cleanup();
}

bool RuntimeServer::listensocket_bind() {
    for (size_t i = 0; i < _config.listen.size(); ++i) {
        int port = _config.listen[i];
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            std::cerr << "Failed to create socket for port " << port << ": " << strerror(errno) << std::endl;
            cleanup();
            return false;
        }

        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            std::cerr << "Failed to set SO_REUSEADDR for port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }

        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            std::cerr << "Failed to set non-blocking mode for port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }

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

        _socketFds.push_back(sockfd);
        _port2socket[port] = sockfd;
    }
    return true;
}

bool RuntimeServer::startListening() {
    for (size_t i = 0; i < _socketFds.size(); ++i) {
        if (listen(_socketFds[i], SOMAXCONN) == -1) {
            std::cerr << "Failed to listen on fd " << _socketFds[i] << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
    return true;
}

void RuntimeServer::cleanup() {
    for (size_t i = 0; i < _socketFds.size(); ++i) {
        if (_socketFds[i] != -1)
            close(_socketFds[i]);
    }
    _socketFds.clear();
    _port2socket.clear();
}

bool RuntimeServer::isListeningOnPort(int port) const {
    return _port2socket.find(port) != _port2socket.end();
}

int RuntimeServer::getSocketForPort(int port) const {
    std::map<int, int>::const_iterator it = _port2socket.find(port);
    if (it == _port2socket.end())
        return -1;
    return it->second;
}

LocationConfig* RuntimeServer::findMatchingLocation(const std::string& path) {
    LocationConfig* bestMatch = NULL;
    size_t bestLength = 0;

    for (size_t i = 0; i < _config.locations.size(); ++i) {
        LocationConfig& location = _config.locations[i];
        const std::string& locationPath = location.path;
        bool matched = false;

        if (!locationPath.empty()) {
            if (path == locationPath)
                matched = true;
            else if (path.compare(0, locationPath.size(), locationPath) == 0) {
                if (locationPath[locationPath.size() - 1] == '/' || path.size() == locationPath.size() || path[locationPath.size()] == '/')
                    matched = true;
            }
        }

        if (matched && locationPath.size() > bestLength) {
            bestLength = locationPath.size();
            bestMatch = &location;
        }
    }
    return bestMatch;
}

bool RuntimeServer::matchesServerName(const std::string& hostHeader) const {
    if (_config.serverName.empty())
        return true;

    std::string host = hostHeader;
    std::string::size_type colonPos = host.find(':');
    if (colonPos != std::string::npos)
        host = host.substr(0, colonPos);

    for (size_t i = 0; i < _config.serverName.size(); ++i) {
        const std::string& serverName = _config.serverName[i];
        if (serverName == "_" || host == serverName)
            return true;
    }
    return false;
}