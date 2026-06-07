#ifndef WEB_SERVER_HPP
# define WEB_SERVER_HPP

# include "../config/config.hpp"
# include "client_connec.hpp"

# include <map>
# include <string>
# include <sys/select.h>
# include <vector>

class WebServer {
    private:
        Config _config;
        std::vector<RuntimeServer*> _servers;
        std::set<int> _listenPorts; // Store unique listen ports for socket creation
        std::vector<int> _listensocketFds;
        std::map<int, int> _fd2port;
        std::map<int, std::vector<RuntimeServer*> > _port2servers;
        std::map<int, ClientConnection*> _clientConnections;
        bool _listensocketBound;
        bool _running;
        fd_set _readFds;
        fd_set _writeFds;
        int _maxFd;

        bool validateConfig();
        bool createListenSockets();
        bool startListening();
        bool createRuntimeServer();
        bool setupPortMapping();
        void updateMaxFd();
        void handleNewConnection(int serverFd);
        void handleClientRequest(int clientFd);
        void handleClientResponse(int clientFd);
        void closeClientConnection(int clientFd);
        void resetConnection(ClientConnection* conn);
        bool parseHttpRequest(ClientConnection* conn);
        void buildHttpResponse(ClientConnection* conn);

        void cleanup();

    public:
        WebServer();
        ~WebServer();

        bool initializeWebserv(const std::string& configFile);
        bool start();
        void stop();
        void run();

        bool isRunning() const { return _running; }
        const Config& getConfig() const;
        size_t getServerCount() const;
        RuntimeServer* hostRouting(const std::string& hostHeader, int port);
        std::string getLastError() const;
};

#endif