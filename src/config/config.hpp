#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "./ServerConfig.hpp"
#include "./LocationConfig.hpp"
#include <stdexcept>


struct Config{
    std::vector<ServerConfig>   servers;
    Config() {};
    void    addServer(const ServerConfig& server){
        servers.push_back(server);
    }
    size_t  getServerCount() const {
        return servers.size();
    }
    const ServerConfig& getServer(size_t index) const{
        if (index >= servers.size())
            throw std::out_of_range("Server index out of range");
        return servers[index];
    }
    void    clear(){
        servers.clear();
    }
    bool    empty() const{
        return servers.empty();
    }
};

#endif