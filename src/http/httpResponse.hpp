#ifndef HTTPRESPONSE
# define HTTPRESPONSE

# include "httpRequest.hpp"

// struct of the response:
// HTTP/1.1 200 OK
// Content-Length: 11
// Content-Type: text/plain

// Hello World

class httpResponse
{
    private:
        std::string _version;
		std::string _status_code;
		std::string _status_message;
        std::map<std::string, std::string> _headers;
        std::string _body;
    public:
        bool buildResponse(const std::string &request);
};

#endif