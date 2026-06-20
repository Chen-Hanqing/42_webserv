#include "server_block.hpp"

#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

ServerBlock::ServerBlock(const ServerConfig& serverBlockConfig) : _config(serverBlockConfig) {}

ServerBlock::~ServerBlock() {}



LocationConfig* ServerBlock::locationRouting(const std::string& path) {
    LocationConfig* bestMatch = NULL;
    size_t bestLength = 0;

    for (size_t i = 0; i < _config.locations.size(); ++i) {
        LocationConfig& location = _config.locations[i];
        const std::string& locationPath = location.path;
        if (locationPath.empty())
            continue;

        // 统一去掉 locationPath 末尾的 '/' 用于前缀比较
        std::string cleanLoc = locationPath;
        if (cleanLoc.size() > 1 && cleanLoc[cleanLoc.size() - 1] == '/')
            cleanLoc.erase(cleanLoc.size() - 1);

        bool matched = false;

        if (path == cleanLoc) {
            matched = true; // 完全相等，例如 path=/directory, cleanLoc=/directory
        } else if (path.size() > cleanLoc.size()
                   && path.compare(0, cleanLoc.size(), cleanLoc) == 0
                   && path[cleanLoc.size()] == '/') {
            matched = true; // 前缀匹配，且紧跟一个 '/'，例如 path=/directory/xxx
        }

        if (matched && locationPath.size() > bestLength) {
            bestLength = locationPath.size();
            bestMatch = &location;
        }
    }
    return bestMatch;
}

bool ServerBlock::matchesServerName(const std::string& hostHeader) const {
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