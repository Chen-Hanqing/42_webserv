#ifndef HTTPRESPONSE
# define HTTPRESPONSE

# include "requestParse.hpp"

// struct of the response:
// HTTP/1.1 200 OK
// Content-Length: 11
// Content-Type: text/plain

// Hello World

// 1. GET + 静态页面        ⭐⭐⭐⭐⭐
// 2. 404 页面    

class httpResponse
{
    private:
		int _status_code;
		std::string _status_message;
        std::string _contentType;
        std::map<std::string, std::string> _headers;
        std::string _body;

    public:
        httpResponse();
        explicit httpResponse(int code);
        
        void setStatusCode(int code);
        void setHeaders(const requestParse& request);
        void setBody(std::string body);
        void setContentType(std::string contentType);

        void addHeadersValue(const std::string& key, const std::string& value);
        std::string getStatusMessage() const;

        std::string buildStatusLine();
        std::string buildHeaders();
        std::string buildResponse(const requestParse& request);
};

#endif