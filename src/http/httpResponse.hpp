#ifndef HTTPRESPONSE
# define HTTPRESPONSE

# include "requestParse.hpp"
# include "../config/ServerConfig.hpp"
# include <fstream>
// struct of the response:
// HTTP/1.1 200 OK
// Content-Length: 11
// Content-Type: text/plain

// Hello World

// 1. GET + 静态页面        ⭐⭐⭐⭐⭐s
// 2. 404 页面    

class httpResponse
{
    private:
		int _status_code;
		std::string _status_message;
        std::string _contentType;
        bool _keepAlive;
        std::map<std::string, std::string> _headers;
        std::string _body;

    public:
        httpResponse();
        explicit httpResponse(int code);
        
        void setStatus(int code);
        void setHeaders();
        void setBody(std::string body);
        void setContentType(std::string contentType);
        void setKeepAlive(bool keepAlive);

        void addHeadersValue(const std::string& key, const std::string& value);
        std::string getStatusMessage() const;
        std::string getStatusMessage(int code) const;

        std::string buildStatusLine();
        std::string buildHeaders();
        std::string buildResponse();

        std::string buildDefaultErrorPage();
        void finalize(const ServerConfig& server);
};

#endif