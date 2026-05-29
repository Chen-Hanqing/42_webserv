class   WebServer{
    private:
        Config config;
        std::vector<ServerInstance> servers;
        std::map<int, std::vector<ServerInstance*> >port2server;
        bool    initialized;
        bool    running;
        std::map<int, ClientConnection*> clientConnection;
        fd_set  readFds, writeFds;
        int maxFd;
        
}