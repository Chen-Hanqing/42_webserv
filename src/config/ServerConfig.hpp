#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP
#include <string>
#include <vector>
#include <map>

#include "./LocationConfig.hpp"

struct ServerConfig{
    std::vector<int> listen;
    std::vector<std::string> serverName;
    size_t  clientMaxBody;
    std::string root;
    std::vector<std::string>    indice;
    std::map<int, std::string>  errorPages;
    std::vector<LocationConfig> locations;

    ServerConfig() : clientMaxBody(1048576) {}

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

    bool    hasLocation(const std::string& path) const {
        for (size_t i = 0; i < locations.size(); i++){
            if (locations[i].path == path)
                return true;
        }
        return false;
    }

    std::string getErrorPage(int code) const
    {
        std::map<int, std::string>::const_iterator it = errorPages.find(code);

        if (it == errorPages.end())
            return "";

        std::string path = it->second;

        if (!path.empty() && path[0] != '/')
            return root + "/" + path;

        return root + path;
    }
};

#endif