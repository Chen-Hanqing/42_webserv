#ifndef CLIENT_CONNEC_HPP 
# define CLIENT_CONNEC_HPP

class   ClientConnection{
    int fd;
    std::string request_buffer;
    std::string response_buffer;

   HttpRequest* request;
   HttpResponse* response;

    bool    _request_complete;
    bool    _response_ready;

    size_t  _bytes_sent;
    time_t  _last_active;
};
#endif
