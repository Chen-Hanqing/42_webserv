#include "httpRequest.hpp"

int main()
{
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    httpRequest req;
    req.parseRequest(raw);

    // HttpResponse res = handle(req);

    // std::cout << res.toString();
}

