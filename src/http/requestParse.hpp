#ifndef REQUESTPARSE
# define REQUESTPARSE

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <sstream>
# include <cstdlib>

enum ValidationResult
{
    OK = 200,
    BAD_REQUEST = 400,
    HTTP_VERSION_NOT_SUPPORTED = 505
};

class requestParse
{
    private:
        std::string _method;
        std::string _rawURI;
        std::string _path;
        std::string _queryString;
        std::map<std::string, std::string> _queryParams;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
        std::string raw;
        size_t _contentLength;
        size_t _headerEnd;
        bool _headersParsed;

        static std::string toLower(std::string s);
    public:
        bool parseRequest();
        bool parseRequestLine(const std::string &requestLine);
        void parseQuery(std::string &str);
        bool parseHeaders(const std::string &headers);
        void parseBody(const std::string &request);
    
        void splitURI(std::string &URI);
        ValidationResult validateRequest() const;
        bool isRequestComplete();
        bool hasHeader(const std::string& key) const;
        
        static std::string trim(const std::string &s);
        std::string urlDecode(const std::string &str);
        std::vector<std::string> splitBySpace(const std::string &requestLine);

        const std::map<std::string, std::string> getHeaders() const;
        std::string getHeader(std::string key) const;
        int getContentLength() const;
        std::string getContentType() const;
        std::string getHost() const;
        std::string getMethod() const;
        std::string getRawURI() const;
        std::string getPath() const;
        std::string getQueryString() const;
        const std::map<std::string,std::string>& getQueryParams() const;
        std::string getVersion() const;
        std::string getBody() const;
        const std::string&  getRaw() const;

        void append(const std::string &data);
};

#endif
