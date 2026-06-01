#ifndef HTTPREQUEST
# define HTTPREQUEST

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <sstream>

class httpRequest
{
    private:
        std::string _method;
        std::string _path;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
    public:
        bool parseRequest(const std::string &request);
        bool parseRequestLine(const std::string &requestLine);
        bool parseHeaders(const std::string &headers);
};

#endif

// validate request
    // if (method : get post delete) 
        // 200 OK
    // else
        // 405 Method Not Allowed
    // http version is 1.1
        // if not, 505 HTTP Version Not Supported
    // if host
        // not, 400 Bad Request