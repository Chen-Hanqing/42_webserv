#include "web_server.hpp"
#include "server_block.hpp"
#include "../config/configparser.hpp"
#include "../http/requestDispatcher.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

bool WebServer::parseHttpRequest(ClientConnection* conn) {
    if (!conn || !conn->_http_request)
        return false;
    return conn->_http_request->parseRequest();
}

//key function recv()
void WebServer::handleClientRequest(int clientFd) {

    // std::cerr << "[DEBUG] handleClientRequest called for fd=" << clientFd << std::endl;

    std::map<int, ClientConnection*>::iterator it = _clientConnections.find(clientFd);
    if (it == _clientConnections.end() || it->second == NULL)
        return;

    ClientConnection* conn = it->second;

    if (!conn->_http_request)
        conn->_http_request = new requestParse();
    if (!conn->_http_response)
        conn->_http_response = new httpResponse();

    char buffer[4096];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        closeClientConnection(clientFd);
        return;
    }
    // std::cerr << "[TIMING] recv " << bytesRead << " bytes, raw.size() now = " 
    //       << (conn->_http_request->getRaw().size() + bytesRead) << std::endl;
    
    conn->_last_active = time(NULL);

    if (!conn->_request_start_logged) {
        // std::cerr << "[TIMING] request started at " << time(NULL) << std::endl;
        conn->_request_start_logged = true; // 需要在 ClientConnection 里加这个标志
    }

    conn->_http_request->append(std::string(buffer, bytesRead));
    // ---- 新增：如果客户端要求 100-continue，且还没发过，立刻发送 ----
    if (conn->_http_request->needsContinueResponse() && !conn->_continueSent)
    {
        const char* continueMsg = "HTTP/1.1 100 Continue\r\n\r\n";
        send(clientFd, continueMsg, strlen(continueMsg), 0);
        conn->_continueSent = true;
    }

    if (!conn->_http_request->isRequestComplete())
        return;

    // std::cerr << "[TIMING] request completed at " << time(NULL) 
    //            << ", body size = " << conn->_http_request->getBody().size() << std::endl;

    conn->_request_complete = true;
    std::cerr << "[DEBUG] request complete, body size = " 
          << conn->_http_request->getBody().size() 
          << ", expected = " << conn->_http_request->getContentLength()
          << std::endl;
    if (!parseHttpRequest(conn)) {
        httpResponse    res(400);
        conn->response_buffer = res.buildResponse();
        conn->_response_ready = true;
        return;
    }

    std::string host = conn->_http_request->getHost();
    conn->_server_block = hostRouting(host, conn->_listen_port);
    if (conn->_server_block)
        conn->_matched_location = conn->_server_block->locationRouting(conn->_http_request->getPath());
    requestDispatcher dispatcher;
    LocationConfig* loc = conn->_matched_location;
    if (loc)
    {
        httpResponse res = dispatcher.dispatch(*conn->_http_request, *conn->_matched_location, conn->_server_block->getConfig());
        conn->_keepAliveForNextRequest = (conn->_http_request->getHeader("Connection") != "close");
        if (loc->hasReturn){
            res.setStatus(loc->returnCode);
            if (!loc->returnUrl.empty())
                res.addHeadersValue("Location", loc->returnUrl);
            conn->response_buffer = res.buildResponse();
            conn->_response_ready = true;
            return;
        }
        conn->response_buffer = res.buildResponse();
    }
    conn->_response_ready = true;
}

//key function send()
void WebServer::handleClientResponse(int clientFd) {
    std::map<int, ClientConnection*>::iterator it = _clientConnections.find(clientFd);
    if (it == _clientConnections.end() || it->second == NULL)
        return;

    ClientConnection* conn = it->second;
    if (!conn || !conn->_response_ready)
        return;

    if (conn->_bytes_sent >= conn->response_buffer.size()) {
        if (conn->_keepAliveForNextRequest) {
            resetConnection(conn);   // 复用连接，准备接收下一个请求
        } else 
            closeClientConnection(clientFd);
        return;
    }

    ssize_t bytesSent = send(clientFd, conn->response_buffer.data() + conn->_bytes_sent,
        conn->response_buffer.size() - conn->_bytes_sent, 0);
    if (bytesSent <= 0) {
        closeClientConnection(clientFd);
        return;
    }

    conn->_bytes_sent += static_cast<size_t>(bytesSent);
    if (conn->_bytes_sent >= conn->response_buffer.size()){
        if (conn->_keepAliveForNextRequest)
            resetConnection(conn);
        else
            closeClientConnection(clientFd);
    }
}
