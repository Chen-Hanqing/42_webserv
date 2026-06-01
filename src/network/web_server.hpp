#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

#include "../config/config.hpp"
#include "client_connec.hpp"

class   WebServer{
    private:
        Config _config;
        std::vector<RuntimeServer*> _servers;
        std::map<int, std::vector<RuntimeServer*> >_port2servers;
        bool listensocket_binded;
        bool running;
        std::map<int, ClientConnection*> _clientConnections;
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
        bool    createRuntimeServer();
        bool    setupPortMapping();
        void    printServerInfo();

        void    cleanup();

    public:
        WebServer();
        ~WebServer();
        bool    listensocket_bind(const std::string& configFile);
        bool    listensocket_bindFromConfig(const Config& cfg);
        bool    start();
        void    stop();
        bool    isRunning() const { return running; }
        const Config& getConfig() const;
        size_t  getServerCount() const;
        RuntimeServer* findServerByHost(const std::string& hostHeader, int port);
        int getPortFromClientSocket(int clientFd);
        void    run();
        std::string getLastError() const;
};
#endif