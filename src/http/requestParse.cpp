#include "requestParse.hpp"

void requestParse::parseBody(const std::string &request)
{
    std::cout << "RAW SIZE = "
              << request.size()
              << std::endl;
    size_t headEnd = request.find("\r\n\r\n");
    std::cout << "HEAD END = "
              << headEnd
              << std::endl;
    if (headEnd == std::string::npos)
        return ;
    _body = request.substr(headEnd + 4);
    std::cout
<< "BODY PARSED SIZE = "
<< _body.size()
<< std::endl;
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

void requestParse::append(const std::string &data)
{
    raw += data;
    std::cout << "RAW SIZE NOW " << raw.size() << std::endl;
    if (!_headersParsed)
    {
        _headerEnd = raw.find("\r\n\r\n");
        if (_headerEnd != std::string::npos)
        {
            _headersParsed = true;

            std::string header = raw.substr(0, _headerEnd);
            size_t  firstCRLF = header.find("\r\n");
            if (firstCRLF != std::string::npos){
                std::string headerOnly = header.substr(firstCRLF + 2);
                parseHeaders(headerOnly);
            }
            bool    ok = parseHeaders(header);
            std::cout << "PARSE HEADER RESULT IS " << ok << std::endl;
            std::cout << "HEADER COUNT " << _headers.size() << std::endl;

            if (hasHeader("content-length"))
                _contentLength = getContentLength();
            else
                _contentLength = 0;
            std::cout << "CONTENTLENGTH" << _contentLength << std::endl;
        }
    }

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


bool requestParse::parseRequest()
{
    if (raw.empty()){
        std::cout << "EMPTY REQUEST" << std::endl;
        return false;
    }
    std::cout << "ENTER parseRequest" << std::endl;
    std::string requestLine = getRequestLine(raw);

    std::cout << "REQUEST LINE = [" << requestLine << "]" << std::endl;

    if (requestLine.empty())
    {
        std::cout << "REQUEST LINE EMPTY" << std::endl;
        return false;
    }

    if (!parseRequestLine(requestLine))
    {
        std::cout << "REQUEST LINE PARSE FAILED" << std::endl;
        return false;
    }

    std::cout << "REQUEST LINE OK" << std::endl;

    std::string headersStr = getHeadersStr(raw);

    if (!parseHeaders(headersStr))
    {
        std::cout << "HEADER PARSE FAILED" << std::endl;
        return false;
    }

    std::cout << "HEADER OK" << std::endl;

    parseBody(raw);

    std::cout << "BODY OK" << std::endl;

    return true;
}