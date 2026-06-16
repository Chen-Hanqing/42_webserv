#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

class   CGIHandler{
public:
    bool    execute(const requestParse& req, const std::string& interpreter, const std::string& scriptPath,
                std::map<std::string, std::string>& headers, std::string& body);
private:
    bool    parseCGIOutput(const std::string& raw, std::map<std::string, std::string>& headers, std::string& body);
};

#endif