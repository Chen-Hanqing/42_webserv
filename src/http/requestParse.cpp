#include "requestParse.hpp"

void requestParse::parseBody(const std::string &request)
{
    size_t headEnd = request.find("\r\n\r\n");
    if (headEnd == std::string::npos)
        return ;
    _body = request.substr(headEnd + 4);
}

// get headers line by line, seperate by ":"
bool requestParse::parseHeaders(const std::string &headersStr)
{
    std::string line;
    std::istringstream iss(headersStr);
    std::string key;
    std::string value;
    while(std::getline(iss, line))
    {
        size_t pos = line.find(":");
        if (pos == std::string::npos)
            return false;
        key = toLower(trim(line.substr(0, pos)));
        value = trim(line.substr(pos + 1));
        _headers[key] = value;
    }
    return true;
}

static std::string getHeadersStr(const std::string &request)
{
    size_t startPos = request.find("\r\n");
    size_t endPos = request.find("\r\n\r\n");
    if (startPos == std::string::npos || endPos == std::string::npos)
        return "";
    return (request.substr(startPos + 2, endPos - startPos - 2));
}

void requestParse::parseQuery(std::string &str)
{
    size_t i = 0;
    while (i < str.size())
    {
        size_t eqPos = str.find('=', i);
        if (eqPos == std::string::npos)
            return ;

        size_t andPos = str.find('&', eqPos);
        std::string key = str.substr(i, eqPos - i);
        std::string value;
        if (andPos == std::string::npos)
            value = str.substr(eqPos + 1);
        else
            value = str.substr(eqPos + 1, andPos - eqPos - 1);
        key = urlDecode(key);
        value = urlDecode(value);
        _queryParams[key] = value;
        if (andPos == std::string::npos)
            break ;
        i = andPos + 1;
    }
}

bool requestParse::parseRequestLine(const std::string &requestLine) // need to check line
{
    std::vector<std::string> parts;

    parts = splitBySpace(requestLine);
    if (parts.size() != 3)
        return false;
    _method = parts[0];
    _rawURI = parts[1];
    splitURI(_rawURI);
    _path = urlDecode(_path);
    parseQuery(_queryString);
    _version = parts[2];
    return true;
}

static std::string getRequestLine(const std::string &request)
{
    size_t pos = request.find("\r\n");
    if (pos == std::string::npos)
        return "";
    return (request.substr(0, pos));
}

bool requestParse::parseRequest(const std::string &request)
{
    if (request.empty())
        return false;
    std::string requestLine = getRequestLine(request);
    if (requestLine.empty())
        return false;
    if (!parseRequestLine(requestLine))
        return false;
    std::string headersStr = getHeadersStr(request);
    if (!parseHeaders(headersStr))
        return false;
    parseBody(request);
    return true;
}