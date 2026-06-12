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

LocationConfig requestDispatcher::findLocation(const std::string& path)
{
    LocationConfig best;
    size_t best_len = 0;

    for (size_t i = 0; i < _locations.size(); i++)
    {
        const std::string& loc_path = _locations[i].path;

        if (path.compare(0, loc_path.length(), loc_path) == 0 &&
            (path.length() == loc_path.length() || path[loc_path.length()] == '/'))
        {
            if (loc_path.length() > best_len)
            {
                best = _locations[i];
                best_len = loc_path.length();
            }
        }
    }
    if (best.path.empty())
    {
        best.path = "/";
        best.root = "./www";
    }
    return best;
}

httpResponse requestDispatcher::dispatch(const requestParse& req)
{
    httpResponse res;
    
    ValidationResult result = req.validateRequest();
    if (result != OK)
    {
        std::cout << "Code from request parse: " << result << std::endl;
        res = httpResponse(result);
    }
    else
    {
        std::string path = req.getPath();
        LocationConfig location = findLocation(path);
        std::string method = req.getMethod();
        if (!isMethodAllowed(method, location))
            res = httpResponse(405);
        else if (method == "GET")
            res = handlerGet(req, location);
        else if (method == "POST")
            res = handlerPost(req, location);
        else if (method == "DELETE")
            res = handlerDelete(req, location);

        // // ------- test ---------
        // std::cout << "raw path: " << path << std::endl;
        // std::cout << "LOCATION: " << location.path << std::endl;
        // std::cout << "ROOT: " << location.root << std::endl;
        // // ------- test ---------

    }
    
    bool keepAlive = (req.getHeader("Connection") != "close");
    res.setKeepAlive(keepAlive);

    return res;
}

void requestDispatcher::addLocation(const LocationConfig& loc)
{
    _locations.push_back(loc);
}
