#include "requestDispatcher.hpp"

/*
1. findLocation
2. build path = root + uri
3. if ".." → 403
4. if directory:
       if index exists → append index
       else → 403
5. if file not exist → 404
6. open file
7. read body
8. set Content-Type
9. return 200
*/

bool endsWith(const std::string& str, const std::string& suffix)
{
    if (suffix.size() > str.size())
        return false;

    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string getContentType(const std::string& path)
{
    if (endsWith(path, ".html"))
        return "text/html";
    if (endsWith(path, ".css"))
        return "text/css";
    if (endsWith(path, ".js"))
        return "application/javascript";
    if (endsWith(path, ".png"))
        return "image/png";
    if (endsWith(path, ".jpg"))
        return "image/jpeg";
    return "application/octet-stream";
}

httpResponse buildAutoIndex(const std::string& path,
                            const std::string& uri)
{
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return httpResponse(404);

    std::stringstream body;
    body << "<html>\n";
    body << "<head><title>Index of " << uri << "</title></head>\n";
    body << "<body>\n";
    body << "<h1>Index of " << uri << "</h1>\n";
    body << "<hr>\n";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        // skip . and ..
        if (name == ".")
            continue;
        body << "<a href=\"";
        if (uri[uri.size() - 1] != '/')
            body << uri << "/" << name;
        else
            body << uri << name;
        body << "\">" << name << "</a><br>\n";
    }

    body << "<hr>\n";
    body << "</body>\n";
    body << "</html>\n";
    closedir(dir);

    httpResponse res(200);
    res.setContentType("text/html");
    res.setBody(body.str());

    return res;
}

/*.  handlerGet
    build path (router)
    if the file under the path can be opened
        yes, read file
            get and set body to response
        no, return 404 response
*/
httpResponse requestDispatcher::handlerGet(const requestParse& req, LocationConfig& location)
{
    std::string pathRequest = req.getPath();

    // ---- 特判：请求路径正好是 location 前缀去掉结尾斜杠 ----
    // 例如 location.path = "/directory/", pathRequest = "/directory"
    if (!location.path.empty()
        && location.path[location.path.size() - 1] == '/')
    {
        std::string locNoSlash = location.path.substr(0, location.path.size() - 1);
        if (pathRequest == locNoSlash)
        {
            httpResponse res(301);
            res.addHeadersValue("Location", pathRequest + "/");
            res.setBody("");
            return res;
        }
    }
    std::string path = buildPath(pathRequest, location);
    std::string interpreter;
    if (isCGI(path, location, interpreter))
        return buildCGIResponse(req, interpreter, path);
    if (path.find("..") != std::string::npos)
        return httpResponse(403);

    struct stat s;
    if (!pathExists(path, s))
        return httpResponse(404);

    if (isDir(s) && (pathRequest.empty() || pathRequest[pathRequest.size() - 1] != '/')){
        httpResponse    res(301);
        res.addHeadersValue("Location", pathRequest + "/");
        res.setBody("");
        return res;
    }

    if (isDir(s))
    {
        bool found = false;
        for (size_t i = 0; i < location.index.size(); i++)
        {
            std::string test = path + "/" + location.index[i];
            struct stat ts;
            if (pathExists(test, ts) && isFile(ts))
            {
                path = test;
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (location.autoindex)
                return buildAutoIndex(path, pathRequest);
            return httpResponse(404);
        }
    }

    struct stat fs;
    if (!pathExists(path, fs) || !isFile(fs))
        return httpResponse(404);

    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return httpResponse(404);

    std::stringstream body_ss;
    body_ss << file.rdbuf();

    httpResponse res(200);
    res.setBody(body_ss.str());
    res.setContentType(getContentType(path));
    return res;
}
