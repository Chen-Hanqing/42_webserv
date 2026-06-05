#include "requestParse.hpp"
#include "httpResponse.hpp"
#include "requestDispatcher.hpp"

int main()
{
    std::string raw =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    requestParse req;
    req.parseRequest(raw);

    // print out request
    std::cout << "method: " << req.getMethod() 
            << "\n_URI: " << req.getURI()
            << "\nversion: " << req.getVersion()
            << "\nheaders: ";
    for (std::map<std::string, std::string>::const_iterator it = req.getHeader().begin();
     it != req.getHeader().end();
     ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "\n";

    ValidationResult result = req.validateRequest();
    requestDispatcher rd;
    httpResponse res = rd.dispatch(req, result);
    // httpResponse res(result);
    std::cout << res.buildResponse(req) << std::endl;
}

