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

    public:
        RuntimeServer(const ServerConfig& blockConfig);
        ~RuntimeServer();

        const ServerConfig& getConfig() const { return _config; }
        LocationConfig* locationRouting(const std::string& path);
        bool matchesServerName(const std::string& hostHeader) const;
};

#endif