#ifndef CLIENT_CONNECTION_HPP
# define CLIENT_CONNECTION_HPP

# include <ctime>
# include <string>

# include "../http/requestParse.hpp"
# include "../http/httpResponse.hpp"

struct LocationConfig;
class ServerBlock;

class ClientConnection {
    public:
        int fd;
        int _listen_port;
        std::string response_buffer;

        requestParse* _http_request;
        httpResponse* _http_response;

        bool _request_complete;
        bool _response_ready;
        bool _continueSent;
        bool _request_start_logged;
        bool _keepAliveForNextRequest;

        ServerBlock* _server_block;
        LocationConfig* _matched_location;
        size_t _bytes_sent;
        time_t _last_active;

        ClientConnection();
        ClientConnection(int socket_fd);
        ~ClientConnection();
};

#endif
