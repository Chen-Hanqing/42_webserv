#include "httprequest.hpp"

void httpRequest::setMethod(const std::string &m);
void httpRequest::setPath(onst std::string &p);
void httpRequest::setVersion(onst std::string &v);
void httpRequest::setHeader(const std::string &key, const std::string &value);


void parseRequestLine(cosnt std::string &buffer, httpRequest req)
{
    // TBD
}

httpRequest parseRequest(const std::string &request_buffer)
{
    httpRequest req;

    parseRequestLine(request_buffer, req);
    parseHeaders(request_buffer, req);
    // parseBody(buffer, req); // 第一周先空着
    return req;
}

