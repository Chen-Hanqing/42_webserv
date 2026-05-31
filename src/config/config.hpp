#ifndef CONFIG_HPP
# define CONFIG_HPP

struct LocationConfig{
    std::string path;
    std::string root;
    std::string alias;
    std::vector<std::string>    index;
    std::vector<std::string>    allowMethods;
    bool    autoindex;
    size_t  clientMax;
    LocationConfig() : autoindex(false), clientMax(static_cast<size_t>(-1)) {}
    LocationConfig(const std::string& locationPath) : path(locationPath), audoindex(false), clientMax(static_cast<size_t>(-1)) {}    
};

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