#include"httpRequest.hpp"

static std::vector<std::string> splitBySpace(const std::string &requestLine)
{
    std::vector<std::string> results;
    std::string part;

    std::istringstream iss(requestLine);
    while(iss >> part)
        results.push_back(part);
    return results;
}

bool httpRequest::parseRequestLine(const std::string &requestLine) // need to check line
{
    std::vector<std::string> parts;

    parts = splitBySpace(requestLine);
    if (parts.size() != 3)
        return false;
    method = parts[0];
    path = parts[1];
    version = parts[2];
    return true;
}

static std::string getRequestLine(const std::string& request)
{
    size_t pos = request.find("\r\n");
    if (pos == std::string::npos)
        return "";
    return (request.substr(0, pos));
}

bool httpRequest::parseRequest(const std::string &request)
{
    if (request.empty())
        return false;
    std::string requestLine = getRequestLine(request);
    if (requestLine.empty())
        return false;
    if (!parseRequestLine(requestLine))
        return false;
    // parseHeaders(request_buffer, req);
    // parseBody(buffer, req); // 第一周先空着
    return true;
}

