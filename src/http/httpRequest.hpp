#ifndef HTTPREQUEST
# define HTTPREQUEST

# include <iostream>
# include <string>
# include <map>

class httpRequest
{
    private:
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
    public:
        void setMethod(const std::string &m);
        void setPath(onst std::string &p);
        void setVersion(onst std::string &v);
        void setHeader(const std::string &key, const std::string &value);
}

httpRequest parseRequest(const std::string &request_buffer);

#endif