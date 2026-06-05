# include "httpResponse.hpp"

// struct of the response:
// HTTP/1.1 200 OK
// Content-Length: 11
// Content-Type: text/plain

// Hello World

httpResponse::httpResponse() : _status_code(0)
{
}

httpResponse::httpResponse(int code) : _status_code(code)
{
}

void httpResponse::setBody(std::string body)
{
	_body = body;
}

void httpResponse::setContentType(std::string contentType)
{
	_contentType = contentType;
}

std::string httpResponse::getStatusMessage() const
{
	switch(_status_code)
	{
		case 200: return "OK";
		case 400: return "Bad Request";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 505: return "Http Version Not Supported";
		default: return "Unknown Status";
	}
}

void httpResponse::setStatusCode(int code)
{
    _status_code = code;
}

std::string httpResponse::buildStatusLine()
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << _status_code << " " << getStatusMessage() << "\r\n";
	return oss.str();
}

std::string httpResponse::buildHeaders()
{
	std::ostringstream oss;

	std::map<std::string, std::string>::const_iterator it;

	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	return oss.str();
}

std::string getCurrentTime()
{
	time_t now = time(0);
	struct tm* gmt = gmtime(&now);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	return std::string(buffer);
}

void httpResponse::addHeadersValue(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

// without redir : Location: /newpage
// POST 上传成功：Content-Type: text/html Content-Length: xxx
// Delete Content-Length: 0
void httpResponse::setHeaders(const requestParse& request)
{
	 _headers.clear();
	addHeadersValue("Date", getCurrentTime());
    addHeadersValue("Server", "webserv");

    addHeadersValue("Content-Length",
        std::to_string(_body.size()));

    addHeadersValue("Content-Type", _contentType);

    if (request.getHeader("Connection") == "keep-alive")
        addHeadersValue("Connection", "keep-alive");
    else
        addHeadersValue("Connection", "close");
}

std::string httpResponse::buildResponse(const requestParse& request)
{
	std::string response;
	setHeaders(request);

	std::string statusLine = buildStatusLine();
	std::string headersStr = buildHeaders();
	response = statusLine + headersStr + "\r\n" + _body;
	return response;
}