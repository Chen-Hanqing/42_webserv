#include "CGIProcess.hpp"

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
    if (pid == 0){
        childProcess(stdinPipe, stdoutPipe, interpreter, scriptPath, envp);
        exit(1);
    }
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
    // execve("/usr/bin/env", argv, envp);  // 临时调试用，让 env 命令打印所有环境变量
    exit(1);
}

bool CGIProcess::writeAllToPipe(int fd, const std::string& input)
{
    size_t totalWritten = 0;
    while (totalWritten < input.size())
    {
        ssize_t n = write(fd, input.c_str() + totalWritten, input.size() - totalWritten);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            if (errno == EPIPE)
                return false;  // 子进程已经关闭读端，正常情况（子进程提前退出不读完 body）
            return false;
        }
        totalWritten += static_cast<size_t>(n);
    }
    return true;
}

bool CGIProcess::parentProcess(pid_t pid, int stdinPipe[2], int stdoutPipe[2], 
    const std::string& input, std::string& output)
{
    std::cerr << "[DEBUG] parentProcess input.size() = " << input.size() << std::endl;

    close(stdinPipe[0]);
    close(stdoutPipe[1]);

    fcntl(stdinPipe[1], F_SETFL, O_NONBLOCK);
    fcntl(stdoutPipe[0], F_SETFL, O_NONBLOCK);

    size_t totalWritten = 0;
    bool stdinClosed = false;
    bool stdoutClosed = false;

    if (input.empty())
    {
        close(stdinPipe[1]);
        stdinClosed = true;
    }

    while (!stdoutClosed)
    {
        fd_set readSet, writeSet;
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);

        int maxFd = stdoutPipe[0];
        FD_SET(stdoutPipe[0], &readSet);

        if (!stdinClosed)
        {
            FD_SET(stdinPipe[1], &writeSet);
            if (stdinPipe[1] > maxFd)
                maxFd = stdinPipe[1];
        }

        timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int ret = select(maxFd + 1, &readSet, &writeSet, NULL, &tv);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            break;
        }
        if (ret == 0)
        {
            kill(pid, SIGKILL);
            int status;
            waitpid(pid, &status, 0);
            return false;
        }

        if (!stdinClosed && FD_ISSET(stdinPipe[1], &writeSet))
        {
            size_t chunk = std::min(input.size() - totalWritten, (size_t)65536);
            ssize_t n = write(stdinPipe[1], input.c_str() + totalWritten, chunk);
            if (n > 0)
                totalWritten += static_cast<size_t>(n);
            else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                close(stdinPipe[1]);
                stdinClosed = true;
            }

            if (totalWritten >= input.size())
            {
                close(stdinPipe[1]);
                stdinClosed = true;
            }
        }

        if (FD_ISSET(stdoutPipe[0], &readSet))
        {
            char buffer[65536];
            ssize_t n = read(stdoutPipe[0], buffer, sizeof(buffer));
            if (n > 0)
                output.append(buffer, n);
            else if (n == 0)
            {
                close(stdoutPipe[0]);
                stdoutClosed = true;
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                close(stdoutPipe[0]);
                stdoutClosed = true;
            }
        }
    }

    if (!stdinClosed)
        close(stdinPipe[1]);

    int status;
    time_t start = time(NULL);
    while (true) {
        pid_t ret = waitpid(pid, &status, WNOHANG);
        if (ret == pid)
            break;
        if (time(NULL) - start > 5) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            return false;
        }
        usleep(10000);
    }
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

// bool    CGIProcess::parentProcess(pid_t pid, int stdinPipe[2], int stdoutPipe[2], 
//             const std::string& input, std::string& output)
// {
//     close(stdinPipe[0]);
//     close(stdoutPipe[1]);
//     if (!input.empty())
//         writeAllToPipe(stdinPipe[1], input);
//     close(stdinPipe[1]);
//     char    buffer[4096];
//     ssize_t bytes;
//     while ((bytes = read(stdoutPipe[0], buffer, sizeof(buffer))) > 0)
//         output.append(buffer, bytes);
//     close(stdoutPipe[0]);
//     int status;
//     time_t  start = time(NULL);
//     while (true){
//         pid_t   ret = waitpid(pid, &status, WNOHANG);
//         if (ret == pid)
//             break;
//         if (time(NULL) - start > 5){
//             kill(pid, SIGKILL);
//             waitpid(pid, &status, 0);
//             return false;
//         }
//         usleep(10000);
//     }
//     return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
// }