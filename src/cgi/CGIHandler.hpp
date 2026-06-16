#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

class   CGIHandler{
    public:
        bool    execute(
            const std::string& interpreter,
            const std::string& script, 
            const requestParse& request,
            std::string& output);
}

#endif