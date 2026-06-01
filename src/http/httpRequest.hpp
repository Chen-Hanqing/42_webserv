#ifndef HTTPREQUEST
# define HTTPREQUEST

# include <iostream>
# include <string>
# include <map>
# include <vector>

class httpRequest
{
    private:
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
    public:
        bool parseRequest(const std::string &request);
        bool parseRequestLine(const std::string &requestLine);
};

#endif