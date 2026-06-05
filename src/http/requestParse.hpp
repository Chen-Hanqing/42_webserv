#ifndef REQUESTPARSE
# define REQUESTPARSE

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <sstream>

enum ValidationResult
{
    OK = 200,
    BAD_REQUEST = 400,
    METHOD_NOT_ALLOWED = 405,
    HTTP_VERSION_NOT_SUPPORTED = 505
};

class requestParse
{
    private:
        std::string _method;
        std::string _URI;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
    public:
        bool parseRequest(const std::string &request);
        bool parseRequestLine(const std::string &requestLine);
        bool parseHeaders(const std::string &headers);
    
        ValidationResult validateRequest();

        std::string getHeader(std::string key) const;
        std::string getMethod() const;
        std::string getURI() const;
        std::string getVersion() const;
        std::string getBody() const;
        const std::map<std::string, std::string>& getHeader() const;
};

#endif
