#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP
#include <string>
#include <vector>
#include <map>

#include "./LocationConfig.hpp"

struct ServerConfig{
    std::vector<int> listen;
    std::vector<std::string> serverName;
    size_t  clientMax;
    std::string root;
    std::vector<std::string>    indice;
    std::map<int, std::string>  errorPages;
    std::vector<LocationConfig> locations;

    ServerConfig() : clientMax(1048576) {}

    void    addListenPort(int port){
        listen.push_back(port);
    }
    void    addServerName(const std::string& name){
        serverName.push_back(name);
    }
    void    addErrorPage(int errorCode, const std::string& page){
        errorPages[errorCode] = page;
    }
    void    addLocation(const LocationConfig& location){
        locations.push_back(location);
    }
};

#endif