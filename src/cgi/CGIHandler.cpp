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
    return parseCGIOutput(rawOutput, headers, body);
}

bool    CGIHandler::parseCGIOutput(const std::string& raw, std::map<std::string, std::string>& headers, std::string& body){
    size_t  pos = raw.find("\r\n\r\n");
    size_t  skip;
    if (pos != std::string::npos)
        skip = 4;
    else{
        pos = raw.find("\n\n");
        if (pos == std::string::npos)
            return false;
        skip = 2;
    }
    std::string headerPart = raw.substr(0, pos);
    body = raw.substr(pos + skip);
    std::stringstream ss(headerPart);
    std::string line;
    while (std::getline(ss, line)){
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        size_t  colon = line.find(':');
        if (colon == std::string::npos)
            continue;
        std::string key = requestParse::trim(line.substr(0, colon));
        std::string value = requestParse::trim(line.substr(colon+1));
        headers[key] = value;
    }
    return true;
}