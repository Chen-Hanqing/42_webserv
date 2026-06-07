#include "client_connec.hpp"

ClientConnection::ClientConnection() : fd(-1), _listen_port(-1), _http_request(NULL), _http_response(NULL),
    _request_complete(false), _response_ready(false), _runtime_server(NULL), _matched_location(NULL),
    _bytes_sent(0), _last_active(time(NULL)) {}

ClientConnection::ClientConnection(int socket_fd) : fd(socket_fd), _listen_port(-1), _http_request(NULL), _http_response(NULL),
    _request_complete(false), _response_ready(false), _runtime_server(NULL), _matched_location(NULL),
    _bytes_sent(0), _last_active(time(NULL)) {}

ClientConnection::~ClientConnection() {
    delete _http_request;
    delete _http_response;
}