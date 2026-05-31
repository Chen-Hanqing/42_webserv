#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

class   WebServer{
    private:
        Config config;
        std::vector<ServerInstance> servers;
        std::map<int, std::vector<ServerInstance*> >portToserver;
        bool initialized;
        bool running;
        std::map<int, ClientConnection*> clientConnections;
        fd_set readFds, writeFds;
        int maxFd;
        void    handleNewConnection(int serverFd);
        void    handleClientRequest(int clientFd);
        void    handleClientResponse(int clientFd);
        void    closeClientConnection(int clientFd);
        void    resetConnection(ClientConnection* conn);
        bool    parseHttpRequest(ClientConnection* conn);
        void    buildHttpResponse(ClientConnection* conn);
        void    updateMaxFd();

        bool    validateConfig();
        bool    createServerInstances();
        bool    setupPortMapping();
        void    printServerInfo();

        void    cleanup();

    public:
        WebServer();
        ~WebServer();
        bool    initialize(const std::string& configFile);
        bool    initializeFromConfig(const Config& cfg);
        bool    start();
        void    stop();
        bool    isRunning() const { return running; }
        const Config& getConfig() const;
        size_t  getServerCount() const;
        ServerInstance* findServerByHost(const std::string& hostHeader, int port);
        int getPortFromClientSocket(int clientFd);
        void    run();
        std::string getLastError() const;
};
#endif