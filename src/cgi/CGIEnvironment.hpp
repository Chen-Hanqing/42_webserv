#ifndef CGIENVIRONMENT_HPP
# define CGIENVIRONMENT_HPP

class   CGIEnvironment{
public:
    char**  buildEnv(const requestParse& req, const std::string& scriptPath);
    void    freeEnv(char** envp); 
};
#endif