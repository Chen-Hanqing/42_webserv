#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "./ServerBlockConfig.hpp"
#include "./LocationConfig.hpp"
#include <stdexcept>


struct Config{
    std::vector<ServerBlockConfig>   servers;
    Config() {};
    void    addServer(const ServerBlockConfig& server){
        servers.push_back(server);
    }
    size_t  getServerCount() const {
        return servers.size();
    }
    const ServerBlockConfig& getServer(size_t index) const{
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