#include "web_server.hpp"

void    WebServer::run(){
    if (!running){

    }
    while (running){
        //add sockets
        for (size_t i = 0; i < _servers.size(); ++i){
            const std::vector<int>& socketFds = _servers[i].getSocketFds();
            for (size_t j = 0; j < socketFds.size(); ++j)
                FD_SET(socketFds[j], &readFds);
        }
        //monitor client connection
        for (std::map<int, ClientConnection*>::iterator it = ClientConnection.begin(); it != ClientConnection.end(); ++it)
        {
            if ()
                FD_SET(fd, &readFds);
            if ()
                FD_SET(fd, &writeFds);
        }

        // select()
        int activity = select(, &readFds, &writeFds, NULL, &timeout);
        // new connection
        for (size_t i = 0; i < _servers.size(); ++i){
            std::vector<int>&   socketFds = _servers[i]->getSocketFds();
            for (size_t j = 0; j < socketFds.size(); j++){
                int newsockFd = socketFds[j];
                if (FD_ISSET(newsockFd, &readFds))
                    handleNewConnection(newsockFd);
            }
        }

        //handle client request and response
        for (std::map<int, ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end();){
            int clientFd = it->first;
            std::map<int, ClientConnection*>::iterator  next_it = it;
            ++next_it; // didnt write the iteration in the loop for safety
            if (FD_ISSET(clientFd, &readFds))
                handleClientRequest(clientFd);
            it = clientConnections.find();

        }
    }
}


static void handleRedirect(ClientConnection* conn, std::string& uri){

}


static bool handleCGIExecution(ClientConnection* conn, const std::string scriptPath, CGIHandler& cgiHandler){

}

static void handleDirRequest(){

}

static void handleGetResponse(){
    if (!isMethodAllowed("GET", conn->_matched_location)){
        conn->response_buffer = conn->_http_response->buildErrorResponse(405, "Method Not Allowed");
        return;
    }
    if (conn->_matched_location && !conn->_matched_location->redirect.empty()){
        handleRedirect(conn);
        return;
    }
    std::string file_path = buildFilePath(conn, uri);
//check for CGI request
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) == 0 && S_ISDIR(file_stat.st_mode)){
        handleDirRequest(conn, file_path, uri);
        return;
    }
    conn->response_buffer = conn->_http_response->buildFileResponse(file_path, *conn->_http_request);
}

static void handlePostResponse(){
    if (!isMethodAllowed("POST", conn->_matched_location)){
        conn->response_buffer =_http_response->buildErrorResponse(405, "Method Not Allowed");
        return;
    }
    std::string file_path = buildFilePath(conn, uri);
    size_t configMax = conn->_runtime_server->getConfig().clientMax;
    if (conn->_matched_location && conn->_matched_location->clientMax != static_cast<size_t>(-1))
        configMax = conn->_matched_location->clientMax;
    if (configMax > 0){
        size_t  requestBodySize = conn->_http_request->getBody().size();
        if (requestBodySize > configMax){
            conn->response_buffer = conn->_http_response->buildErrorResponse(413, "Content Too Large");
            return;
        }
        //check for CGI
        if (conn->_http_request->isMultipartFormData()){
            if (!conn->_http_request->parseMultipartFormData()){
                conn->response_buffer = conn->http_resposne->buildErrorResponse(400, "Bad Request");
                return;
            }
            std::vector<FileUploatd>    files = conn->_http_request->getUploadedFiles();
            std::map<std::string, std::string> form_data = conn->_http_request->getPortFormData();

            if (mkdir(file_path.c_str(), 0755) != 0 && errno != EEXIST)
                conn->response_buffer = conn->_http_response->buildErrorResponse(500, "Internal Server")

        }
    }
}

static void handleDeleteResponse(){

}