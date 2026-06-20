# include "httpResponse.hpp"

// struct of the response:
// HTTP/1.1 200 OK
// Content-Length: 11
// Content-Type: text/plain

// Hello World

httpResponse::httpResponse() 
	: _status_code(0), _keepAlive(false)
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

void httpResponse::setKeepAlive(bool keepAlive)
{
	_keepAlive = keepAlive;
}

std::string httpResponse::getStatusMessage() const
{
    return getStatusMessage(_status_code);
}

std::string httpResponse::getStatusMessage(int code) const
{
    switch (code)
    {
        case 200: return "OK";
        case 201: return "Created";
        case 301: return "Moved Permanently";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 505: return "Http Version Not Supported";
        default:   return "Unknown Status";
    }
}

void httpResponse::setStatus(int code)
{
    _status_code = code;
}

int httpResponse::getStatusCode() const{
    return  _status_code;
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

std::string intToString(int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

// without redir : Location: /newpage
// POST 上传成功：Content-Type: text/html Content-Length: xxx
// Delete Content-Length: 0
void httpResponse::setHeaders()
{
    // 不再 clear()，只补全标准字段，且不覆盖已经手动设置的值
    if (!_headers.count("Date"))
        addHeadersValue("Date", getCurrentTime());
    if (!_headers.count("Server"))
        addHeadersValue("Server", "webserv");

    // Content-Length 必须每次都重新计算，因为 body 可能在这之后变化
    addHeadersValue("Content-Length", intToString(_body.size()));

    if (!_headers.count("Content-Type"))
        addHeadersValue("Content-Type", _contentType);

    if (!_headers.count("Connection"))
    {
        if (_keepAlive)
            addHeadersValue("Connection", "keep-alive");
        else
            addHeadersValue("Connection", "close");
    }
}

std::string httpResponse::buildResponse()
{
	std::string response;
	setHeaders();

	std::string statusLine = buildStatusLine();
	std::string headersStr = buildHeaders();
	response = statusLine + headersStr + "\r\n" + _body;
	return response;
}

std::string httpResponse::buildDefaultErrorPage()
{
    std::string message = getStatusMessage(_status_code);

    std::stringstream ss;

    ss << "<html><body>"
       << _status_code << " " << message
       << "</body></html>";

    return ss.str();
}
void httpResponse::finalize(const ServerConfig& server)
{
    // =========================
    // 1. ERROR PAGE HANDLING
    // =========================
    if (_status_code >= 400 && _body.empty())
    {
        std::string path = server.getErrorPage(_status_code);

        if (!path.empty())
        {
            std::ifstream file(path.c_str(), std::ios::binary);
            if (file.is_open())
            {
                std::stringstream ss;
                ss << file.rdbuf();
                _body = ss.str();
                _contentType = "text/html";
            }
        }

        if (_body.empty())
            _body = buildDefaultErrorPage();

        if (_contentType.empty())
            _contentType = "text/html";
    }

    // =========================
    // 2. DEFAULT CONTENT-TYPE
    // =========================
    if (_contentType.empty())
    {
        if (_body.empty())
            _contentType = "text/plain";
        else
            _contentType = "text/html";
    }

    // =========================
    // 3. CONTENT-LENGTH HEADER
    // =========================
    // IMPORTANT: no member variable, computed directly
    addHeadersValue("Content-Length", intToString(_body.size()));
}


