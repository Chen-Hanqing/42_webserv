bool    CGIProcess::execute(const std::string& interpreter, const std::string& scriptPath,
            char** envp, const std::string& input, std::string& output)
{

    int stdinPipe[2];
    int stdoutPipe[2];

    if (pipe(stdinPipe) < 0)
        return false;
    if (pipe(stdoutPipe) < 0)
    {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        return false;
    }
    pid_t   pid = fork();
    if (pid < 0)
    {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return false;
    }
    if (pid == 0)
        childProcess(stdinPipe, stdoutPipe, interpreter, scriptPath, envp);
    else
        return parentProcess(pid, stdinPipe, stdoutPipe, input, output);
}

void    CGIProcess::childProcess(int stdinPipe[2], int stdoutPipe[2],
             const std::string& interpreter, const std::string& scriptPath, char** envp)
{
    dup2(stdoutPipe[1], STDOUT_FILENO);
    dup2(stdinPipe[0], STDIN_FILENO);
    close(stdinPipe[0]);
    close(stdinPipe[1]);
    close(stdoutPipe[0]);
    close(stdoutPipe[1]);
    char    *argv[3];
    argv[0] = const_cast<char*>(interpreter.c_str());
    argv[1] = const_cast<char*>(scriptPath.c_str());
    argv[2] = NULL;
    execve(interpreter.c_str(), argv, envp);
    exit(1);
}

bool    CGIProcess::parentProcess(pid_t pid, int stdinPipe[2], int stdoutPipe[2], 
            const std::string& input, std::string& output)
{
    close(stdinPipe[0]);
    close(stdoutPipe[1]);
    if (!input.empty())
        write(stdinPipe[1], input.c_str(), input.size());
    close(stdinPipe[1]);
    char    buffer[4096];
    ssize_t bytes;
    while ((bytes = read(stdoutPipe[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytes);
    close(stdoutPipe[0]);
    int status;
    waitpid(pid, &status, 0);
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
