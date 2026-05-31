#include "client_connec.hpp"

ClientConnection::ClientConnection(): fd(-1), _bytes_sent(0), _request_complete(false),
    _response_ready(false), _last_active(time(NULL)), 
    _http_request(NULL), _http_response(NULL), _runtime_server(NULL), _matched_location(NULL){}

ClientConnection::ClientConnection(int socket_fd) : fd(socket_fd), _bytes_sent(0),
    _request_complete(false), _response_ready(false), _last_active(time(NULL)), 
   _http_request(NULL), _http_response(NULL), _runtime_server(NULL), _matched_location(NULL){}

ClientConnection::~ClientConnection(){
    if (_http_request)
        delete _http_request;
    if (_http_response)
        delete _http_response;
}