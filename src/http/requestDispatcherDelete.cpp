#include "requestDispatcher.hpp"

/*
1. findLocation
2. build path
3. security check (..)
4. if directory → 403 (通常)
5. if file not exist → 404
6. unlink(file)
7. return 200 or 204
*/
httpResponse requestDispatcher::handlerDelete(const requestParse& req, LocationConfig& location)
{
    std::string pathRequest = req.getPath();
    std::string path = buildPath(pathRequest, location);
    if (path.find("..") != std::string::npos)
        return httpResponse(403);
    struct stat s;
    if (!pathExists(path, s))
        return httpResponse(404);
    if (isDir(s))
        return httpResponse(403);
    if ((!isFile(s)))
        return httpResponse(404);
    if (unlink(path.c_str()) != 0)
        return httpResponse(500);
    return httpResponse(204);
}
