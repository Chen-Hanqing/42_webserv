char**   buildEnv(const requestParse& req, const std::string& scriptPath){
    std::vector<std::string>    vars;

    vars.push_back("REQUEST_METHOD=" + req.getMethod());
    vars.push_back("QUERY_STRING=" + req.getQueryString());
    vars.push_back("CONTENT_LENGTH=" + req.getHeader("content-length"));
    vars.push_back("CONTENT_TYPE=" + req.getHeader("content-type"));
    vars.push_back("SCRIPT_NAME=" + scriptPath);
    vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    //transform vector to char**
    char**  envp = new char*[vars.size() + 1];
    for (size_t i = 0; i < vars.size(); ++i)
        envp[i] = strdup(vars[i].c_str());
    envp[vars.size()] = NULL;
    return envp;
}

void    CGIEnvironment::freeEnv(char** envp){
    int i = 0;
    while (envp[i]){
        free(envp[i]);
        i++;
    }
    delete[] envp;
}