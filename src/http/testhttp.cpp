#include "requestParse.hpp"
#include "httpResponse.hpp"
#include "requestDispatcher.hpp"

int main()
{
    std::string rawRequestGet =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: test\r\n"
        "Accept: */*\r\n"
        "\r\n";

    std::string rawRequestPost = 
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
       " User-Agent: test-client\r\n"
        "Accept: */*\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 27\r\n"
        "\r\n"
        "name=linna&lang=cplusplus&ecole=42\r\n";

    std::string rawRequestDelete =
        "DELETE /file.txt HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: test-client\r\n"
        "Accept: */*\r\n"
        "\r\n";

    std::cout << "===== RAW REQUEST =====\n";
    std::cout << rawRequestPost << "\n";

    requestParse req;
    req.parseRequest(rawRequestPost);

    requestDispatcher dispatcher;
    httpResponse res = dispatcher.dispatch(req);

    std::cout << "===== RESPONSE =====\n";
    std::cout << res.buildResponse() << std::endl; 

    return 0;
}


// int main()
// {
//     std::cout << "===== Router Test =====" << std::endl;

//     requestDispatcher dispatcher;

//     LocationConfig loc1;
//     loc1.path = "/images";
//     loc1.root = "./www/images";

//     LocationConfig loc2;
//     loc2.path = "/images/icons";
//     loc2.root = "./www/icons";

//     LocationConfig loc3;
//     loc3.path = "/";
//     loc3.root = "./www";

//     dispatcher.addLocation(loc1);
//     dispatcher.addLocation(loc2);
//     dispatcher.addLocation(loc3);

//     std::string uri = "/images/icons/logo.png";

//     LocationConfig match = dispatcher.findLocation(uri);

//     std::cout << "matched path: "
//               << match.path << std::endl;

//     std::cout << "matched root: "
//               << match.root << std::endl;

//     std::cout << "\n===== Request Parse Test =====" << std::endl;

//     std::string raw =
//         "GET /index.html HTTP/1.1\r\n"
//         "Host: localhost\r\n"
//         "\r\n";

//     requestParse req;
//     req.parseRequest(raw);

//     std::cout << "method: "
//               << req.getMethod() << std::endl;

//     std::cout << "uri: "
//               << req.getURI() << std::endl;

//     std::cout << "version: "
//               << req.getVersion() << std::endl;

//     std::cout << "headers:" << std::endl;

//     for (std::map<std::string, std::string>::const_iterator it =
//             req.getHeader().begin();
//          it != req.getHeader().end();
//          ++it)
//     {
//         std::cout << it->first
//                   << ": "
//                   << it->second
//                   << std::endl;
//     }

//     std::cout << "\n===== Validation Test =====" << std::endl;

//     ValidationResult result = req.validateRequest();

//     std::cout << "validation result: "
//               << result
//               << std::endl;

//     std::cout << "\n===== Dispatch Test =====" << std::endl;

//     httpResponse res =
//         dispatcher.dispatch(req, result);

//     std::cout << res.buildResponse(req)
//               << std::endl;

//     return 0;
// }