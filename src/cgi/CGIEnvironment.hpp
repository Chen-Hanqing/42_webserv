#ifndef CGIENVIRONMENT_HPP
# define CGIENVIRONMENT_HPP

#include "../network/web_server.hpp"

class   CGIEnvironment{
public:
    static char**  buildEnv(const requestParse& req, const std::string& scriptPath);
    static void    freeEnv(char** envp); 
};
#endif