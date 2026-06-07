#include "network/web_server.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./webserv <config_file>" << std::endl;
        return 1;
    }

    WebServer server;

    if (!server.initializeWebserv(argv[1]))
    {
        std::cerr << "Configuration error" << std::endl;
        return 1;
    }

    if (!server.start())
    {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    std::cout << "Webserv started" << std::endl;

    server.run();

    return 0;
}