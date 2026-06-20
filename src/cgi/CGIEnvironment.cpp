#include "CGIEnvironment.hpp"

char**   CGIEnvironment::buildEnv(const requestParse& req, const std::string& scriptPath){
    std::vector<std::string>    vars;

    vars.push_back("REQUEST_METHOD=" + req.getMethod());
    vars.push_back("QUERY_STRING=" + req.getQueryString());
    vars.push_back("CONTENT_LENGTH=" + req.getHeader("Content-Length"));
    vars.push_back("CONTENT_TYPE=" + req.getHeader("Content-Type"));
    vars.push_back("PATH_INFO=/");

    // vars.push_back("SCRIPT_NAME=" + req.getPath());
    vars.push_back("SCRIPT_FILENAME=" + scriptPath);
    // PATH_INFO 应该等于 scriptPath 去掉开头的 "." (如果有的话)，保留斜杠


    vars.push_back("SERVER_NAME=" + req.getHost());

    vars.push_back("SERVER_SOFTWARE=webserv");
    vars.push_back("REDIRECT_STATUS=200");
    vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    vars.push_back("SERVER_PROTOCOL=HTTP/1.1");

    vars.push_back("HTTP_HOST=" + req.getHost());


    const std::map<std::string, std::string>& headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it)
    {
        std::string key = it->first;
        // key 目前是小写（你的 parseHeaders 存的是小写），需要转大写并把 - 换成 _
        for (size_t i = 0; i < key.size(); ++i)
        {
            if (key[i] == '-')
                key[i] = '_';
            else
                key[i] = std::toupper(key[i]);
        }
        if (key == "HOST")
            continue; // 已经单独处理过 HTTP_HOST，避免重复
        if (key == "CONTENT_LENGTH" || key == "CONTENT_TYPE")
            continue;
        vars.push_back("HTTP_" + key + "=" + it->second);
    }
    //transform vector to char**
    char**  envp = new char*[vars.size() + 1];
    for (size_t i = 0; i < vars.size(); ++i)
    {
        envp[i] = strdup(vars[i].c_str());
        // std::cerr << "[ENV] [" << envp[i] << "]" << std::endl;  // 加这行
    }
    envp[vars.size()] = NULL;
    return envp;
}

void    CGIEnvironment::freeEnv(char** envp){
    if (!envp)
        return;
    int i = 0;
    while (envp[i]){
        free(envp[i]);
        i++;
    }
    delete[] envp;
}