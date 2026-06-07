#include "runtime_server.hpp"

#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

RuntimeServer::RuntimeServer(const ServerBlockConfig& serverBlockConfig) : _config(serverBlockConfig) {}

RuntimeServer::~RuntimeServer() {}



LocationConfig* RuntimeServer::locationRouting(const std::string& path) {
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