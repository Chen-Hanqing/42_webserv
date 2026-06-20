#include "requestDispatcher.hpp"

/*
check client_max_body_size
  ↓
parse body
  ↓
route decision:
      ├── static upload
      ├── CGI
      └── reject (405 / 404 / 413)
  ↓
build response
*/

httpResponse requestDispatcher::handlerPost(const requestParse& req, LocationConfig& location)
{
    std::string pathRequest = req.getPath();

    // 1. body size check
    if (req.getBody().size() > location.clientMaxBody)
        return httpResponse(413);

    // 2. build path
    std::string path = buildPath(pathRequest, location);
    std::cerr << "[DEBUG] handlerPost computed path = [" << path << "]" << std::endl;

    std::string interpreter;
    bool cgiMatched = isCGI(path, location, interpreter);
    std::cerr << "[DEBUG] isCGI result = " << cgiMatched << ", interpreter = [" << interpreter << "]" << std::endl;

    if (cgiMatched)
        return buildCGIResponse(req, interpreter, path);

    // 3. security check (nginx style safe guard)
    if (path.find("..") != std::string::npos)
        return httpResponse(403);

    // 4. if directory → nginx behavior: reject POST
    struct stat s;
    if (pathExists(path, s) && isDir(s))
        return httpResponse(403);

    // 5. ensure parent directory exists
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos)
    {
        std::string dir = path.substr(0, pos);
        if (dir.find("..") != std::string::npos)
            return httpResponse(403);
        struct stat ds;
        if (!pathExists(dir, ds) || !isDir(ds))
            return httpResponse(404);
    }

    // 6. write file
    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return httpResponse(500);
    file << req.getBody();
    file.close();

    // 7. response 201
    httpResponse res(201);
    std::string locationHeader;
    if (pathRequest.size() >= location.path.size())
        locationHeader = pathRequest.substr(location.path.size());
    else
        locationHeader = pathRequest; // 防御性兜底，避免越界
    res.addHeadersValue("Location", locationHeader);
    return res;
}
