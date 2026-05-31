RuntimeServer::RuntimeServer(const ServerConfig& serverConfig) : config(serverConfig) {}

RuntimeServer::~RuntimeServer(){
    cleanup();
}

bool    RuntimeServer::initialize(){
    //create socket for each listening port
    for (size_t i = 0; i < config.listen.size(); i++){
        int port = config.listen[i];
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1){
            std::cerr << "Failed to create socket for port " << port << ": " << strerror(errno) << std::endl;
            cleanup();
            return false;
        }
        // set socket option
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
            std::cerr << "Failed to set SO_REUSEADDR for port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }
        // set non-blocking mode
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1){
            std::cerr << "Failed to set non-blocking mode for port " << port << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }
        // bind port
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
            std::cerr << "Failed to bind to port " << port << ": " << strerror(errno) << std::endl;
            close(sockfd);
            cleanup();
            return false;
        }
        socketFds.push_back(sockfd);
        port2socket[port] = sockfd;
        std::cout << "Socket created and bound to port " << port << std::endl;
    }
    return true;
}

bool    RuntimeServer::startListening(){
    for (size_t i = 0; i < socketFds.size(); ++i){
        int sockfd = socketFds[i];
        int port = config.listen[i];
        if (listen(sockfd, SOMAXCONN) == -1){
            std::cerr << "Failed to listen on port " << port << ": " << strerror(errno) << std::endl;
            return false;
        }
        std::cout << "Listening on port " << port << std::endl;
    }
    return true;
}

void    RuntimeServer::cleanup(){
    for (size_t i = 0; i < socketFds.size(); ++i){
        int sockfd = socketFds[i];
        if (sockfd != -1){
            close(sockfd);
            std::cout << "Closed socket fd: " << sockfd << std::endl;
        }
    }
    socketFds.clear();
    port2socket.clear();
    std::cout << "RuntimeServer cleanup completed" << std::endl;
}

bool    RuntimeServer::isListeningOnPort(int port) const{
    return port2socket.find(port) != port2socket.end();
}

bool    RuntimeServer::getSocketForPort(int port) const{
    std::map<int, int>::const_iterator  it = port2socket.find(port);
    return (it != port2socket.end()) ? it->second : -1;
}

LocationConfig* RuntimeServer::findMatchingLocation(const std::string& path){
    LocationConfig* bestMatch = NULL;
    size_t  maxLength = 0;
    for (size_t i = 0; i < config.locations.size(); ++i){
        LocationConfig& location = config.locations[i];
        bool match = false;
        if (location.path == path)
            match = true;
        else if (location.path.length() > 1 && location.path[location.path.length() - 1] == ''
        && path + '/' == location.path)
            match = true;
        else if (location.path.length() <= path.length() && path.substr(0, location.path.length()) == location.path){
            if (location.path[location.path.length() - 1] == '/' || path.length() == location.path.length() || path[location.path.location()] == '/')
                match = true;
        }
        if (match && location.path.length() > maxLength){
            maxLength = location.path.length();
            bestMatch = &location;
        }
        return bestMatch;
    }
}

bool    RuntimeServer::matchesServerName(const std::string& hostHeader) const {
    if (config.serverName.empty())
        return true;
    std::string host = hostHeader;
    size_t  colonPos = host.find(':');
    if (colonPos != std::string::npos)
        host = host.substr(0, colonPos);
    for (size_t i = 0; i < config.serverName.size(); ++i){
        const std::string& serverName = config.serverName[i];
        if (host == serverName || serverName = "_")
            return true;
    }
    return false;
}