#include "requestParse.hpp"

std::string requestParse::toLower(std::string s)
{
    for(size_t i = 0; i < s.size(); i++)
        s[i] = std::tolower(s[i]); 
    return s;
}

std::string requestParse::trim(const std::string &s)
{
    size_t start = 0;
    size_t end = s.size();

    while (start < end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r'))
        start++;

    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r'))
        end--;

    return s.substr(start, end - start);
}

std::string requestParse::urlDecode(const std::string &str)
{
    std::string result;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == '%' && i + 2 < str.size())
        {
            std::string hex = str.substr(i + 1, 2);
            int value = std::strtol(hex.c_str(), NULL, 16);
            result.push_back(static_cast<char>(value));
            i += 2;
        }
        else if (str[i] == '+')
            result.push_back(' ');
        else
            result.push_back(str[i]);
    }
    return result;
}

void requestParse::splitURI(std::string &URI)
{
    size_t pos = URI.find("?");
    if (pos == std::string::npos)
    {
        _path = URI;
        _queryString = "";
    }
    else
    {
        _path = URI.substr(0, pos);
        _queryString = URI.substr(pos + 1);
    }
}

std::vector<std::string> requestParse::splitBySpace(const std::string &requestLine)
{
    std::vector<std::string> results;
    std::string part;

    std::istringstream iss(requestLine);
    while(iss >> part)
        results.push_back(part);
    return results;
}

ValidationResult requestParse::validateRequest() const
{
    if (_version != "HTTP/1.1")
        return HTTP_VERSION_NOT_SUPPORTED;
    std::map<std::string, std::string>::const_iterator it = _headers.find("host");
    if (it == _headers.end() || it->second.empty())
        return BAD_REQUEST;
    return OK;
}

// bool requestParse::isRequestComplete()
// {
//     if (!_headersParsed){
//         size_t headerEnd = raw.find("\r\n\r\n");
//         if (headerEnd == std::string::npos)
//             return false;
    
//         std::string headerPart = raw.substr(0, headerEnd);
//         size_t pos = headerPart.find("Content-Length:");
//         if (pos == std::string::npos){
//             _contentLength = 0;
//             _bodyStart = headerEnd + 4;
//             _headersParsed = true;
//             return raw.size() >= _bodyStart;
//         }
//         pos += 15;
//         while (pos < headerPart.size() && std::isspace(headerPart[pos]))
//             pos++;
//         size_t end = headerPart.find("\r\n", pos);
//         _contentLength = std::atoi(headerPart.substr(pos, end - pos).c_str());
//         _bodyStart = headerEnd + 4;
//         _headersParsed = true;
//     }
//     return  (raw.size() - _bodyStart)  >= (size_t)_contentLength;
// }

// isRequestComplete 现在只读状态，不做任何解析
bool requestParse::isRequestComplete()
{
    if (!_headersParsed)
        return false;

    if (_isChunked)
        return isChunkedBodyComplete();
    if (raw.size() - _bodyStart < _contentLength)
        return false;

    // 只有真正收完整的那一刻，才执行一次 substr
    if (_body.empty() && _contentLength > 0)
        _body = raw.substr(_bodyStart, _contentLength);

    return true;
}

bool requestParse::hasHeader(const std::string& key) const
{
    return _headers.find(toLower(key)) != _headers.end();
}
