#include "requestParse.hpp"

requestParse::requestParse()
    : _contentLength(0), _headerEnd(0), _bodyStart(0),
      _headersParsed(false), _isChunked(false), _chunkedComplete(false),
      _needsContinueResponse(false), _chunkParsePos(0)
{
}

void requestParse::parseBody(const std::string &request)
{
    if (_isChunked)
        return;
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
        if (line.empty() || line == "\r")
            continue;  // 跳过空行，不要直接失败
        size_t pos = line.find(":");
        if (pos == std::string::npos)
            continue;  // 跳过没有冒号的行，而不是 return false
        key = toLower(trim(line.substr(0, pos)));
        value = trim(line.substr(pos + 1));
        _headers[key] = value;
    }
    return true;
}

void requestParse::append(const std::string &data)
{
    raw += data;
    if (!_headersParsed)
    {
        _headerEnd = raw.find("\r\n\r\n");
        if (_headerEnd != std::string::npos)
        {
            _headersParsed = true;
            _bodyStart = _headerEnd + 4;

            std::string header = raw.substr(0, _headerEnd);
            size_t  firstCRLF = header.find("\r\n");
            if (firstCRLF != std::string::npos){
                std::string headerOnly = header.substr(firstCRLF + 2);
                std::cerr << "[DEBUG] headerOnly = [" << headerOnly << "]" << std::endl;
                bool ok = parseHeaders(headerOnly);
                std::cerr << "[DEBUG] parseHeaders returned: " << ok << std::endl;
            }

            if (hasHeader("transfer-encoding"))
            {
                std::string te = toLower(trim(_headers.at("transfer-encoding")));
                if (te.find("chunked") != std::string::npos)
                    _isChunked = true;
            }

            // ---- 现在 _headers 已经填充完毕，可以安全检查 expect ----
            if (hasHeader("expect")){
                std::string expectVal = toLower(trim(_headers.at("expect")));
                if (expectVal.find("100-continue") != std::string::npos)
                    _needsContinueResponse = true;
            }
            std::cerr << "[DEBUG] hasHeader(content-length) = " << hasHeader("content-length") << std::endl;
            _contentLength = 0;
            if (!_isChunked && hasHeader("content-length"))
            {
                std::string clStr = trim(_headers.at("content-length"));
                // 用 strtoul 而不是 atoi，避免负数/溢出问题，且直接得到 size_t 兼容的值
                _contentLength = static_cast<size_t>(std::strtoul(clStr.c_str(), NULL, 10));
                raw.reserve(_bodyStart + _contentLength);  // 提前预留，避免反复扩容拷贝
            }
        }
    }
}

bool requestParse::isChunkedBodyComplete()
{
    if (_chunkParsePos == 0)
        _chunkParsePos = _bodyStart;  // 第一次调用时初始化解析起点

    while (true)
    {
        size_t lineEnd = raw.find("\r\n", _chunkParsePos);
        if (lineEnd == std::string::npos)
            return false; // chunk size 行还没收完整，等下次再来

        std::string sizeLine = raw.substr(_chunkParsePos, lineEnd - _chunkParsePos);
        size_t semi = sizeLine.find(';');
        if (semi != std::string::npos)
            sizeLine = sizeLine.substr(0, semi);

        size_t chunkSize = static_cast<size_t>(std::strtoul(sizeLine.c_str(), NULL, 16));
        size_t chunkDataStart = lineEnd + 2;

        if (chunkSize == 0)
        {
            size_t finalEnd = raw.find("\r\n", chunkDataStart);
            if (finalEnd == std::string::npos)
                return false; // 结束标记还没收完整

            _chunkedComplete = true;
            // std::cerr << "[CHUNK] complete, final _body.size() = " << _body.size() << std::endl;
            return true;
        }

        // 这个 chunk 的数据 + 结尾 \r\n 还没收完整，等下次再来
        if (raw.size() < chunkDataStart + chunkSize + 2)
            return false;

        // 增量追加到 _body，而不是重建整个 decodedBody
        _body.append(raw, chunkDataStart, chunkSize);

        // std::cerr << "[CHUNK] appended " << chunkSize << " bytes, _body.size() now = " << _body.size() << std::endl;
        // 推进解析位置，下次直接从这里继续，不再重新扫描前面已处理的部分
        _chunkParsePos = chunkDataStart + chunkSize + 2;
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