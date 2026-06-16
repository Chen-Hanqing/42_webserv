#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP
#include <string>
#include <vector>
#include <map>

struct LocationConfig{
    std::string path;
    std::string root;
    std::string alias;
    std::string cgiPass;
    std::map<std::string, std::string>  cgiHandlers;
    std::vector<std::string>    index;
    std::vector<std::string>    allowMethods;
    bool    autoindex;
    size_t  clientMaxBody;
    bool    hasReturn;
    int returnCode;
    std::string returnUrl;
    LocationConfig() : autoindex(false), clientMaxBody(static_cast<size_t>(-1)), hasReturn(false), returnCode(0) {}
    LocationConfig(const std::string& locationPath) : 
        path(locationPath), autoindex(false), clientMaxBody(static_cast<size_t>(-1)), hasReturn(false), returnCode(0) {}    
};

#endif
