#ifndef REQUESTDISPATCHER
# define REQUESTDISPATCHER

# include "httpResponse.hpp"
# include "requestParse.hpp"
# include "LocationConfig.hpp"
# include <fstream>
# include <sys/stat.h>
#include <dirent.h>
#include <sstream>
#include <string>
#include <unistd.h>

/*
1. 接收 request + validate result
2. 如果 validate 不通过 → 直接 return error response
3. 根据 method 分发
4. 调用 Router（找到对应 handler）
5. 调用 MethodHandler 执行
6. 返回 response
*/

class requestDispatcher
{
    private:
        httpResponse handlerGet(const requestParse& req, LocationConfig& location);
        httpResponse handlerPost(const requestParse& req, LocationConfig& location);
        httpResponse handlerDelete(const requestParse& req, LocationConfig& location);

    public:
        httpResponse dispatch(const requestParse& req, LocationConfig location);
        // LocationConfig findLocation(const std::string& uri);
    
        void addLocation(const LocationConfig& loc);

        bool pathExists(const std::string& path, struct stat& s);
        bool isDir(const struct stat& s);
        bool isFile(const struct stat& s);
        std::string buildPath(std::string& pathRequest, LocationConfig& location);
};

#endif
