#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP
#include <string>
#include <vector>
#include <map>

struct LocationConfig{
    std::string path;
    std::string root;
    std::string alias;
    std::vector<std::string>    index;
    std::vector<std::string>    allowMethods;
    bool    autoindex;
    size_t  clientMax;
    LocationConfig() : autoindex(false), clientMax(static_cast<size_t>(-1)) {}
    LocationConfig(const std::string& locationPath) : 
        path(locationPath), autoindex(false), clientMax(static_cast<size_t>(-1)) {}    
};

#endif
