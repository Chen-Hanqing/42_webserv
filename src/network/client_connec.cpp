#include "client_connec.hpp"

ClientConnection::ClientConnection(): fd(-1), bytes_sent(0), request_complete(false),
    response_ready(false), last_active(time(NULL)), 
    http_request(NULL), http_response(NULL), server_instance(NULL), matched_location(NULL){}

ClientConnection::ClientConnection(int socket_fd) : fd(socket_fd), bytes_sent(0),
    request_complete(false), response_ready(false), last_active(time(NULL)), 
    http_request(NULL), http_response(NULL), server_instance(NULL), matched_location(NULL){}

ClientConnection::~ClientConnection(){
    if (http_request)
        delete http_request;
    if (http_response)
        delete http_response;
}