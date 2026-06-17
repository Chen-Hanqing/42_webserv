#ifndef CGIPROCESS_HPP
# define CGIPROCESS_HPP
#include "../network/web_server.hpp"
# include "../cgi/CGIHandler.hpp"
# include "../cgi/CGIEnvironment.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

class   CGIProcess{
public:
    static bool    execute(const std::string& interpreter, const std::string& script,
            char** envp, const std::string& input, std::string& output);
private:
        static void childProcess(int stdinPipe[2], int stdoutPipe[2],
             const std::string& interpreter, const std::string& scriptPath, char** envp);
        static bool parentProcess(pid_t pid, int stdinPipe[2], int stdoutPipe[2], 
            const std::string& input, std::string& output);
};
#endif