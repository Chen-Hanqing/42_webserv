#ifndef CLIENT_CONNECTION_HPP
# define CLIENT_CONNECTION_HPP

# include <ctime>
# include <string>

# include "../http/httpRequest.hpp"
# include "../http/httpResponse.hpp"

struct LocationConfig;
class RuntimeServer;

class ClientConnection {
    public:
        int fd;
        int _listen_port;
        std::string request_buffer;
        std::string response_buffer;

        HttpRequest* _http_request;
        HttpResponse* _http_response;

        bool _request_complete;
        bool _response_ready;

        RuntimeServer* _runtime_server;
        LocationConfig* _matched_location;
        size_t _bytes_sent;
        time_t _last_active;

        ClientConnection();
        ClientConnection(int socket_fd);
        ~ClientConnection();
};

#endif
