#include <unistd.h>
#include <stdio.h>

int main() {
    char *argv[] = {"./cgi_tester", NULL};
    char *envp[] = {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=",
        "CONTENT_TYPE=application/octet-stream",
        "GATEWAY_INTERFACE=CGI/1.1",
        "PATH_INFO=/",
        "PATH_TRANSLATED=./YoupiBanane/youpi.bla",
        "QUERY_STRING=",
        "REMOTE_ADDR=127.0.0.1",
        "REMOTE_PORT=46528",
        "REQUEST_METHOD=GET",
        "REQUEST_SCHEME=http",
        "REQUEST_URI=/directory/youpi.bla",
        "SCRIPT_NAME=/directory/youpi.bla",
        "SCRIPT_FILENAME=./YoupiBanane/youpi.bla",
        "SERVER_NAME=localhost",
        "SERVER_PORT=8080",
        "SERVER_PROTOCOL=HTTP/1.1",
        "SERVER_SOFTWARE=webserv/0.1",
        "HTTP_HOST=localhost:8080",
        "HTTP_USER_AGENT=curl/8.11.1",
        "HTTP_ACCEPT=*/*",
        NULL
    };
    execve("./cgi_tester", argv, envp);
    perror("execve failed");
    return 1;
}
