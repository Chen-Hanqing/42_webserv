void    WebServer::handleClientRequest(int clientFd){
    std::map<int, ClientConnection*>::iterator it = clientConnections.find(clientFd);
    if (it == clientConnections.end()) return;
    ClientConnection* conn = it->second;
    if (!conn) return;
    char buffer[4096];
    ssize_t bytesRead = recv(cliendFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0){
        if (bytesRead == 0)
            std::cout << "Client disconnected: fd=" << clientFd << std::endl;
        closeClientConnection(clientFd);
        return;
    } else {
        buffer[bytesRead] = '\0';
        conn->request_buffer += buffer;
        conn->_last_active = time(NULL);
    }
    if (!conn->_http_request)
        conn->_http_request = new HttpRequest();
    if (!conn->_http_response)
        conn->_http_response = new HttpResponse();
    trimValidateRequestBuffer(conn->request_buffer);
    if (conn->request_buffer.empty())
        return;
    RequestStatus status = conn->_http_request->isRequestComplete(conn->request_buffer);
    if (status == _request_complete){
        conn->_request_complete = true;
        if (parseHttpRequest(conn)){
            std::string host = conn->_http_request->getHost();
            int port = getPortFromClientSocket(clientFd);
            if (port == -1)
                conn->_runtime_server = servers.empty() ? NULL : servers[0];
            else
                conn->_runtime_server = findServerByHost(host, port);
            std::string uri = conn->_http_request->getURI();
            conn->_matched_location = conn->_runtime_server->findMatchingLocation(uri);
            buildHttpResponse(conn);
            conn->_response_ready = true;
        }
        else {
            conn->_http_response->resultToStatusCode(conn->_http_request->getValidationStatus());
            conn->response_buffer = conn->_http_response->buildErrorResponse(conn->_http_response->getStatusCode(), "TBU", *conn->_http_request);
            conn->_response_ready = true;
        }
    } else if (status == REQUEST_TOO_LARGE){
        conn->response_buffer = conn->_http_response->buildErrorResponse(413, "Content Too Large", *conn->_http_request);
        conn->_response_ready = true;
    } else if (status == INVALID_REQUEST){
        conn->response_buffer = conn->_http_response->buildHttpResponse(400, "Bad Request", *conn->_http_request);
        conn->_response_ready = true;
    }
}


void    WebServer::handleClientResponse(int clientFd){
    std::map<int, ClientConnection*>::iterator it = clientConnections.find(clientFd);
    if (it == clientConnections.end()) return;
    ClientConnection* conn = it->second;
    if (!conn || !conn->_response_ready) return;
    size_t  remaining = conn->response_buffer.size() - conn->_bytes_sent;
    ssize_t bytesSent = seend(clientFd, data, remaining, 0);
    if (bytesSent > 0){
        conn->_bytes_sent += bytesSent;
        std::cout << "Sent " << bytesSent << " bytes to fd=" << clientFd << std::endl;
    } else if (bytesSent <= 0){
        std::cerr << "send() failed: " << std::endl;
        closeClientConnection(cliendFd);
    }
}