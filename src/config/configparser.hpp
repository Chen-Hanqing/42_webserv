#ifndef CONFIG_PARSER_HPP
# define CONFIG_PARSER_HPP

#include "config.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class   ConfigParser{
    private:
        std::string content;
        std::vector<TOKEN>  tokens;
        size_t  currentTokenIndex;
        size_t  currentLine;
        size_t  currentColumn;

        bool    tokenize(const std::string& configContent);
        void    skipWhitespace(const std::string& content, size_t& pos);
        Token   getNextToken();
        bool    isWordChar(char c);
        bool    isDigit(char c);
        std::string readWord();
        std::string readNumber();
        std::string readString();
        void    skipComment();

        bool    parseConfig(Config& config);
        bool    parseServer(ServerConfig& server);
        bool    parseLocation(LocationConfig& server);
        bool    parseServerDirective(ServerConfig& server);
        bool    parseLocationDirective(LocationConfig& location);

        Token   currenToken();
        Token   peekToken(size_t offset = 1);
        void    consumeToken();
        bool    expectToken(TokenType expectedType);
}
#endif