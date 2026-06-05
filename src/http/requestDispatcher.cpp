#include "requestDispatcher.hpp"

/*
1. 接收 request + validate result
2. 如果 validate 不通过 → 直接 return error response
3. 根据 method 分发
4. 调用 Router（找到对应 handler）
5. 调用 MethodHandler 执行
6. 返回 response
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

/*.  handlerGet
    build path (router)
    if the file under the path can be opened
        yes, read file
            get and set body to response
        no, return 404 response
*/
httpResponse requestDispatcher::handlerGet(const requestParse& req)
{
    std::string uri = req.getURI();
    if (uri == "/")
        uri = "/index.html";
    std::string path;
    path = "./www" + uri;
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return httpResponse(404);
    std::stringstream body_ss;
    body_ss << file.rdbuf();

    httpResponse res(200);
    res.setBody(body_ss.str());
    res.setContentType(getContentType(path));
    return res;
}

httpResponse requestDispatcher::dispatch(const requestParse& req, ValidationResult result)
{
    if (req.getMethod() == "GET")
        return handlerGet(req);
    // else if (req.getMethod() == "POST")
    //     return handlerPost(req);
    // else if (req.getMethod() == "DELETE")
    //     return handlerDelet(req);
    return httpResponse(result);
}
