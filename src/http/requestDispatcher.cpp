#include "requestDispatcher.hpp"

std::string requestDispatcher::buildPath(std::string& pathRequest, LocationConfig& location)
{
    std::string remaining = pathRequest.substr(location.path.length());
    std::string path = location.root + '/' + remaining;
    // // ----------- for test ----------- 
    // std::cout << "FINAL PATH: " << path << std::endl;
    // // ----------- for test ----------- 
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
    res.setKeepAlive(keepAlive);

    res.finalize(server);   // ⭐ error_page 在这里统一处理
    return res;
}

std::string  requestDispatcher::getExtension(const std::string& path){
    size_t  pos = path.rfind(',');
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

    std::map<std::string,std::string>::iterator it =
        location.cgiHandlers.find(ext);

    if (it == location.cgiHandlers.end())
        return false;

    interpreter = it->second;

    return true;
}

httpResponse    requestDispatcher::buildCGIResponse(const requestParse& req, const std::string& interpreter, const std::string& scriptPath){
    httpResponse    res;
    std::map<std::string, std::string>  headers;
    std::string body;
    if (!CGIHandler::execute(req, interpreter, scriptPath, headers, body)){
        res.setStatus(500);
        return  res;
    }
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

// ----------- repeat, to be deleted -----------
// LocationConfig requestDispatcher::findLocation(const std::string& path)
// {
//     LocationConfig best;
//     size_t best_len = 0;

//     for (size_t i = 0; i < _locations.size(); i++)
//     {
//         const std::string& loc_path = _locations[i].path;

//         if (path.compare(0, loc_path.length(), loc_path) == 0 &&
//             (path.length() == loc_path.length() || path[loc_path.length()] == '/'))
//         {
//             if (loc_path.length() > best_len)
//             {
//                 best = _locations[i];
//                 best_len = loc_path.length();
//             }
//         }
//     }
//     if (best.path.empty())
//     {
//         best.path = "/";
//         best.root = "./www";
//     }
//     return best;
// }
// ----------- repeat, to be deleted -----------

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
        // LocationConfig location = findLocation(path);
        std::string method = req.getMethod();
        if (!isMethodAllowed(method, location))
        {
            res.setStatus(405);
            return finalizeResponse(res, server, req);
            // res = httpResponse(405);
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
        // std::cout << "raw path: " << path << std::endl;
        // std::cout << "LOCATION: " << location.path << std::endl;
        // std::cout << "ROOT: " << location.root << std::endl;
        // // ------- test ---------

    }
    return finalizeResponse(res, server, req);
}

