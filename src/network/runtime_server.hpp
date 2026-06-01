#ifndef RUNTIME_SERVER_HPP
# define RUNTIME_SERVER_HPP

# include "../config/ServerConfig.hpp"
# include "../config/LocationConfig.hpp"

# include <cerrno>
# include <cstdio>
# include <fcntl.h>
# include <iostream>
# include <map>
# include <string>
# include <sys/socket.h>
# include <vector>

class RuntimeServer {
    private:
        ServerConfig _config;
        std::vector<int> _socketFds;
        std::map<int, int> _port2socket;

    public:
        RuntimeServer(const ServerConfig& serverConfig);
        ~RuntimeServer();

        bool listensocket_bind();
        bool startListening();
        void cleanup();

        const ServerConfig& getConfig() const { return _config; }
        const std::vector<int>& getSocketFds() const { return _socketFds; }
        bool isListeningOnPort(int port) const;
        int getSocketForPort(int port) const;
        LocationConfig* findMatchingLocation(const std::string& path);
        bool matchesServerName(const std::string& hostHeader) const;
};

#endif