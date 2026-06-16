#include "requestParse.hpp"
#include "httpResponse.hpp"
#include "requestDispatcher.hpp"

int main()
{
    // =========================
    // 1. Fake ServerConfig
    // =========================
    ServerConfig server;
    server.root = "./www";

    server.addErrorPage(404, "/errors/404.html");
    server.addErrorPage(403, "/errors/403.html");
    server.addErrorPage(405, "/errors/405.html");

    // =========================
    // 2. Fake LocationConfig
    // =========================
    LocationConfig location;

    location.path = "/";
    location.root = "./www";
    location.autoindex = false;

    location.index.push_back("index.html");

    // =========================
    // 3. Test cases
    // =========================
    std::vector<std::string> tests;

    // ---- normal request (200)
    tests.push_back(
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    // ---- 404 test (file not exist)
    tests.push_back(
        "GET /not_exist.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    // ---- 405 test (method not allowed)
    tests.push_back(
        "POST /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    // ---- 403 test (force forbidden path traversal)
    tests.push_back(
        "GET /../../etc/passwd HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    // =========================
    // 4. Run tests
    // =========================
    requestDispatcher dispatcher;

    for (size_t i = 0; i < tests.size(); i++)
    {
        std::cout << "\n=========================\n";
        std::cout << "TEST #" << i + 1 << "\n";
        std::cout << "=========================\n";

        std::cout << tests[i] << "\n";

        requestParse req;
        req.parseRequest(tests[i]);

        // ⚠️ 关键：你现在 dispatcher 需要 server + location
        httpResponse res = dispatcher.dispatch(req, location, server);

        std::cout << "----- RESPONSE -----\n";
        std::cout << res.buildResponse() << "\n";
    }

    return 0;
}