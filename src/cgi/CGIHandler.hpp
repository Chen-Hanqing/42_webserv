#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "../network/web_server.hpp"
# include "../cgi/CGIEnvironment.hpp"
# include "../cgi/CGIProcess.hpp"

class   CGIHandler{
public:
    static bool    execute(const requestParse& req, const std::string& interpreter, const std::string& scriptPath,
                std::map<std::string, std::string>& headers, std::string& body);
private:
    static bool    parseCGIOutput(const std::string& raw, std::map<std::string, std::string>& headers, std::string& body);
    static bool looksLikeChunked(const std::string& body);
    static  std::string decodeChunkedBody(const std::string& raw);

};

#endif