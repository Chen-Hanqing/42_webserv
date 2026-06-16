#include "CGIHandler.hpp"

static char**   buildEnv(const requestParse& req){
    std::vector<std::string>    env;

    env.push_back("REQUEST_METHOD=" + req.getMethod());
    env.push_back("QUERY_STRING=" + req.getQueryString());
    env.push_back("CONTENT_LENGTH=" + req.getHeader("content-length"));
    env.push_back("CONTENT_TYPE=" + req.getHeader("content-type"));

}

bool    CGIHandler::execute(const std::string& interpreter, const std::string& script,
            const requestParse& request, std::string& output)
{
    int stdoutPipe[2];
    if (pipe(stdoutPipe) < 0)
        return false;
    pid_t   pid = fork();
    if (pid < 0)
        return false;
    if (pid == 0){
        dup2(stdoutPipe[1], STDOUT_FILENO);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        char    *argv[3];
        argv[0] = const_cast<char*>(interpreter.c_str());
        argv[1] = const_cast<char*>(scriptPath.c_str());
        argv[2] = NULL;
        char**  envp = buildEnv(request);
        execve(interpreter.c_str(), argv, envp);
        freeEnv(envp);
        exit(1);
    }
    close(stdoutPipe[1]);
    char    buffer[4096];
    ssize_t bytes;
    while ((bytes = read(stdoutPipe[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytes);
    close(stdoutPipe[0]);
    waitpid(pid, NULL, 0);
    return true;
}