class   RuntimeServer{
    private:
        ServerConfig config;
        std::vector<int>    socketFds;
        std::map<int, int>  port2socket;
    public:
        RuntimeServer(const ServerConfig& serverConfig);
        ~RuntimeServer();
        bool    initialize();
        bool    startListening();
        void    cleanup();
        const   ServerConfig& getConfig() const { return config; }
        const   std::vector<int>&   getSocketFds() const { return socketFds; }
        bool    isListeningOnPort(int port) const;
        int getSocketForPort(int port) const;
        LocationConfig* findMatchingLocation(const std::string& path);
        bool    matchesServerName(const std::string& hostHeader) const;
};

#endif