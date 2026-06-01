#ifndef CLIENT_CONNEC_HPP 
# define CLIENT_CONNEC_HPP

#include <string>
#include <ctime>
#include "../http/httpRequest.hpp"
#include "../http/httpResponse.hpp"
struct LocationConfig;

class   ClientConnection{
    int fd;
    std::string request_buffer;
    std::string response_buffer;

   HttpRequest* _http_request;
   HttpResponse* _http_response;

    bool    _request_complete;
    bool    _response_ready;

    RuntimeServer*  _runtime_server;
    LocationConfig*    _matched_location;
    size_t  _bytes_sent;
    time_t  _last_active;
};
#endif
