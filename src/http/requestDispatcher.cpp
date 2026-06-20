#include "requestDispatcher.hpp"

// std::string requestDispatcher::buildPath(std::string& pathRequest, LocationConfig& location)
// {
//     //std::string remaining = pathRequest.substr(location.path.length());
//     std::string path = location.root + '/' + pathRequest;
//     // // ----------- for test ----------- 
//     //std::cout << "FINAL PATH: " << path << std::endl;
//     // // ----------- for test ----------- 
//     return path;
// }

std::string requestDispatcher::buildPath(std::string& pathRequest, LocationConfig& location)
{
    std::string remaining;

    if (pathRequest.size() >= location.path.size()
        && pathRequest.compare(0, location.path.size(), location.path) == 0)
        remaining = pathRequest.substr(location.path.size());
    else
        remaining = pathRequest;

    if (!remaining.empty() && remaining[0] == '/')
        remaining = remaining.substr(1);

    std::string root = location.root;
    if (!root.empty() && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);

    std::string path = root;
    if (!remaining.empty())
        path += "/" + remaining;

    return path;
}

bool requestDispatcher::pathExists(const std::string& path, struct stat& s)
{
    return (stat(path.c_str(), &s) == 0);
}

bool requestDispatcher::isDir(const struct stat& s)
{
    return S_ISDIR(s.st_mode);
}

bool requestDispatcher::isFile(const struct stat& s)
{
    return S_ISREG(s.st_mode);
}

bool isMethodAllowed(std::string& method, LocationConfig& location)
{
    if (location.allowMethods.empty())
        return true;
    for (size_t i = 0; i < location.allowMethods.size(); i++)
    {
        if (method == location.allowMethods[i])
            return true;
    }
    return false;
}

httpResponse requestDispatcher::finalizeResponse(httpResponse res, const ServerConfig& server, const requestParse& req)
{
    bool keepAlive = (req.getHeader("Connection") != "close");
    int status = res.getStatusCode();
    if (status == 400 || status == 405 || status == 413 || status >= 500)
        keepAlive = false;
    res.setKeepAlive(keepAlive);

    res.finalize(server);
    return res;
}

std::string  requestDispatcher::getExtension(const std::string& path){
    size_t  pos = path.rfind('.');
    if (pos == std::string::npos)
        return "";
    return path.substr(pos);
}


bool requestDispatcher::isCGI(
    const std::string& path,
    LocationConfig& location,
    std::string& interpreter)
{
    std::string ext = getExtension(path);

    for (std::map<std::string,std::string>::iterator it =
            location.cgiHandlers.begin();
            it != location.cgiHandlers.end();
            ++it)
    {
        std::cout
            << "[" << it->first << "] => "
            << it->second
            << std::endl;
    }

    std::map<std::string,std::string>::iterator it =
        location.cgiHandlers.find(ext);

    if (it == location.cgiHandlers.end())
        return false;

    interpreter = it->second;

    return true;
}

httpResponse    requestDispatcher::buildCGIResponse(const requestParse& req, const std::string& interpreter, const std::string& scriptPath)
{
    httpResponse    res;
    std::map<std::string, std::string>  headers;
    std::string body;
    // // ------- test ---------
    // (void) interpreter;
    // (void) scriptPath;
    // (void) req;
    // // ------- test ---------
    std::cout << "CGI PATH: " << scriptPath << std::endl;
    if (!CGIHandler::execute(req, interpreter, scriptPath, headers, body))
    {
        // std::cerr << "[DEBUG] CGIHandler::execute returned false (exec failed or non-zero exit or timeout)" << std::endl;
        res.setStatus(500);
        return  res;
    }
    // std::cerr << "[DEBUG] CGI raw headers parsed:" << std::endl;
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
        std::cerr << "  [" << it->first << "] = [" << it->second << "]" << std::endl;
    // std::cerr << "[DEBUG] CGI body size = " << body.size() << std::endl;
    res.setStatus(200);
    if (headers.count("Content-Type"))
        res.setContentType(headers["Content-Type"]);
    else
        res.setContentType("text/html");
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it){
        if (it->first == "Content-Type")
            continue;
        if (it->first == "Status")
            continue;
        res.addHeadersValue(it->first, it->second);
    }
    if (headers.count("Status")){
        std::stringstream   ss(headers["Status"]);
        int code;
        ss >> code;
        res.setStatus(code);
    }
    res.setBody(body);
    return res;
}

httpResponse requestDispatcher::dispatch(const requestParse& req, LocationConfig& location, const ServerConfig& server)
{
    httpResponse res;
    
    ValidationResult result = req.validateRequest();
    if (result != OK)
    {
        res.setStatus(result);
        return finalizeResponse(res, server, req);
    }
    else
    {
        std::string path = req.getPath();
        std::string method = req.getMethod();
        if (!isMethodAllowed(method, location))
        {
            res.setStatus(405);
            return finalizeResponse(res, server, req);
        }
        else if (method == "GET")
            res = handlerGet(req, location);
        else if (method == "POST")
            res = handlerPost(req, location);
        else if (method == "DELETE")
            res = handlerDelete(req, location);
        else
            res.setStatus(501);

        // // ------- test ---------
        std::cout << "HOST HEADER: " << req.getHost() << std::endl;
        std::cout << "MATCHED SERVER NAMES: ";
        for (size_t i = 0; i < server.serverName.size(); ++i)
            std::cout << server.serverName[i] << " ";
        std::cout << std::endl;
        std::cout << "raw path: " << path << std::endl;
        std::cout << "LOCATION: " << location.path << std::endl;
        std::cout << "ROOT: " << location.root << std::endl;
        std::cout << "LOCATION ROOT = " << location.root << std::endl;
        // // // ------- test ---------

    }
    return finalizeResponse(res, server, req);
}

