#include "requestParse.hpp"

static std::string trim(const std::string &s)
{
    size_t start = 0;
    size_t end = s.size();

    while (start < end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r'))
        start++;

    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r'))
        end--;

    return s.substr(start, end - start);
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
        key = trim(line.substr(0, pos));
        value = trim(line.substr(pos + 1));
        _headers[key] = value;
    }
    return true;
}

static std::string getHeadersStr(const std::string& request)
{
    size_t startPos = request.find("\r\n");
    size_t endPos = request.find("\r\n\r\n");
    if (startPos == std::string::npos || endPos == std::string::npos)
        return "";
    return (request.substr(startPos + 2, endPos - startPos - 2));
}

static std::vector<std::string> splitBySpace(const std::string &requestLine)
{
    std::vector<std::string> results;
    std::string part;

    std::istringstream iss(requestLine);
    while(iss >> part)
        results.push_back(part);
    return results;
}

bool requestParse::parseRequestLine(const std::string &requestLine) // need to check line
{
    std::vector<std::string> parts;

    parts = splitBySpace(requestLine);
    if (parts.size() != 3)
        return false;
    _method = parts[0];
    _URI = parts[1];
    _version = parts[2];
    return true;
}

static std::string getRequestLine(const std::string& request)
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

    // parseBody(buffer, req); // 第一周先空着

    return true;
}

// validate request
    // if (method : get post delete) 
        // 200 OK
    // else
        // 405 Method Not Allowed
    // http version is 1.1
        // if not, 505 HTTP Version Not Supported
    // if host
        // not, 400 Bad Request

ValidationResult requestParse::validateRequest()
{
    if(_method != "GET" && _method != "POST" && _method != "DELETE")
        return METHOD_NOT_ALLOWED;
    if (_version != "HTTP/1.1")
        return HTTP_VERSION_NOT_SUPPORTED;
    if (_headers.find("Host") == _headers.end() || _headers["Host"].empty())
        return BAD_REQUEST;
    return OK;
}

// --------------------- getter ---------------------

std::string requestParse::getHeader(std::string key) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end())
        return it->second;
    return "";
}

const std::map<std::string, std::string>& requestParse::getHeader() const
{
    return _headers;
}

std::string requestParse::getMethod() const
{
    return _method;
}

std::string requestParse::getURI() const
{
    return _URI;
}

std::string requestParse::getVersion() const
{
    return _version;
}

std::string requestParse::getBody() const
{
    return _body;
}
