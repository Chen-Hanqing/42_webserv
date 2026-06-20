#include "CGIHandler.hpp"


//check cgi or not, check config, call cgi-environment, call cgi-process, parse cgi output, return results.

bool    CGIHandler::execute(const requestParse& req, const std::string& interpreter, const std::string& scriptPath,
        std::map<std::string, std::string>& headers, std::string& body)
{

    std::string rawOutput;
    char**  envp = CGIEnvironment::buildEnv(req, scriptPath);
    bool    success = CGIProcess::execute(interpreter, scriptPath, envp, req.getBody(), rawOutput);
    CGIEnvironment::freeEnv(envp);
    if (!success)
        return false;
        // ---- 诊断：把原始输出存盘 ----
    std::ofstream dbgFile("/tmp/cgi_raw_output.bin", std::ios::binary);
    dbgFile.write(rawOutput.data(), rawOutput.size());
    dbgFile.close();
    std::cerr << "[DEBUG] raw CGI output size = " << rawOutput.size() << std::endl;

    return parseCGIOutput(rawOutput, headers, body);
}

// bool    CGIHandler::parseCGIOutput(const std::string& raw, std::map<std::string, std::string>& headers, std::string& body){
//     // 只在前 8KB 范围内查找 header/body 分隔符，避免误判大体积 body 内容
//     size_t searchLimit = std::min(raw.size(), (size_t)8192);
//     std::string searchArea = raw.substr(0, searchLimit);

//     size_t pos = searchArea.find("\r\n\r\n");
//     size_t skip;
//     if (pos != std::string::npos)
//         skip = 4;
//     else
//     {
//         pos = searchArea.find("\n\n");
//         if (pos == std::string::npos)
//             return false;
//         skip = 2;
//     }
//     std::string headerPart = raw.substr(0, pos);
//     body = raw.substr(pos + skip);
//     std::stringstream ss(headerPart);
//     std::string line;
//     while (std::getline(ss, line)){
//         if (!line.empty() && line[line.size() - 1] == '\r')
//             line.erase(line.size() - 1);
//         size_t  colon = line.find(':');
//         if (colon == std::string::npos)
//             continue;
//         std::string key = requestParse::trim(line.substr(0, colon));
//         std::string value = requestParse::trim(line.substr(colon+1));
//         headers[key] = value;
//     }
//     return true;
// }

bool CGIHandler::parseCGIOutput(const std::string& raw, std::map<std::string, std::string>& headers, std::string& body)
{
    size_t searchLimit = std::min(raw.size(), (size_t)8192);
    std::string searchArea = raw.substr(0, searchLimit);

    size_t pos = searchArea.find("\r\n\r\n");
    size_t skip;
    if (pos != std::string::npos)
        skip = 4;
    else
    {
        pos = searchArea.find("\n\n");
        if (pos == std::string::npos)
            return false;
        skip = 2;
    }

    std::string headerPart = raw.substr(0, pos);
    std::string rawBody = raw.substr(pos + skip);

    std::stringstream ss(headerPart);
    std::string line;
    while (std::getline(ss, line)){
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;
        std::string key = requestParse::trim(line.substr(0, colon));
        std::string value = requestParse::trim(line.substr(colon+1));
        headers[key] = value;
    }

    // ---- 检测并解码 chunked body ----
    if (!headers.count("Content-Length") && looksLikeChunked(rawBody))
        body = decodeChunkedBody(rawBody);
    else
        body = rawBody;

    return true;
}


bool CGIHandler::looksLikeChunked(const std::string& body)
{
    size_t lineEnd = body.find("\r\n");
    if (lineEnd == std::string::npos || lineEnd == 0)
        return false;
    std::string firstLine = body.substr(0, lineEnd);
    // 检查这一行是否全部由十六进制字符组成（可能带 chunk extension，这里只做基本检查）
    for (size_t i = 0; i < firstLine.size(); ++i)
    {
        if (!std::isxdigit(static_cast<unsigned char>(firstLine[i])))
            return false;
    }
    return true;
}

std::string CGIHandler::decodeChunkedBody(const std::string& raw)
{
    std::string decoded;
    size_t pos = 0;
    while (pos < raw.size())
    {
        size_t lineEnd = raw.find("\r\n", pos);
        if (lineEnd == std::string::npos)
            break;
        std::string sizeLine = raw.substr(pos, lineEnd - pos);
        size_t chunkSize = static_cast<size_t>(std::strtoul(sizeLine.c_str(), NULL, 16));
        size_t dataStart = lineEnd + 2;
        if (chunkSize == 0)
            break; // 结束 chunk
        if (dataStart + chunkSize > raw.size())
            break; // 数据不完整，防御性退出
        decoded.append(raw, dataStart, chunkSize);
        pos = dataStart + chunkSize + 2; // 跳过数据和结尾的 \r\n
    }
    return decoded;
}