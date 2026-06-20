#include "requestParse.hpp"
const std::map<std::string, std::string> requestParse::getHeaders() const
{
    return _headers;
}

std::string requestParse::getHeader(std::string key) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(toLower(key));
    if (it == _headers.end())
        return "";
    return it->second;
}

size_t requestParse::getContentLength() const
{
    if (!hasHeader("content-length"))
        return -1;
    return std::atoi(getHeader("content-length").c_str());
}

std::string requestParse::getContentType() const
{
    return getHeader("content-type");
}

std::string requestParse::getHost() const
{
    return getHeader("host");
}

std::string requestParse::getMethod() const
{
    return _method;
}

std::string requestParse::getRawURI() const
{
    return _rawURI;
}

std::string requestParse::getPath() const
{
    return _path;
}
std::string requestParse::getQueryString() const
{
    return _queryString;
}
const std::map<std::string,std::string>& requestParse::getQueryParams() const
{
    return _queryParams;
}

std::string requestParse::getVersion() const
{
    return _version;
}

std::string requestParse::getBody() const
{
    return _body;
}

const std::string &requestParse::getRaw() const
{
    return raw;
}

bool    requestParse::needsContinueResponse() const{
    return _needsContinueResponse;
}