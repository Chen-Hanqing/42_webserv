#ifndef RUNTIME_SERVER_HPP
# define RUNTIME_SERVER_HPP

#include "../config/ServerConfig.hpp"
#include "../config/LocationConfig.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

class   RuntimeServer{
    private:
        ServerConfig config;
        std::vector<int>    socketFds;
        std::map<int, int>  port2socket; //one client socket for each connection
    public:
        RuntimeServer(const ServerConfig& serverConfig);
        ~RuntimeServer();
        bool    listensocket_bind();
        bool    startListening();
        void    cleanup();
        const   ServerConfig& getConfig() const { return config; }
        const   std::vector<int>&   getSocketFds() const { return socketFds; }
        bool    isListeningOnPort(int port) const;
        int getSocketForPort(int port) const;
        LocationConfig* findMatchingLocation(const std::string& path);
        bool    matchesServerName(const std::string& hostHeader) const;
};

#endif