ConfigParser::ConfigParser() : currentTokenIndex(0), currentLine(1), currentColumn(1) {}

ConfigParser::~ConfigParser() {}

bool    ConfigParser::parseFile(const std::string& filename, Config& config){
    std::ifstream   file(filename.c_str());
    if (!file.is_open()){
        lastError = "Cannot Open File: " + filename;
        return false;
    }
    std::string content(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    file.close();
    return parseString(content, config);
}

bool    ConfigParser::parseString(std::string& configContent, Config& config){
    content = configContent;
    currentTokenIndex = 0;
    currentLine = 1;
    currentColumn = 1;
    tokens.clear();
    config.clear();
    if (!tokenize(configContent))
        return false;
    return parseConfig(config);
}

bool    ConfigParser::parseConfig(Config& config){
    currentTokenIndex = 0;
    while (currentToken().type != TOKEN_EOF){
        if (){
            ServerConfig    server;
            consumeToken();
            if (!expectOpenBrace())
                return false;
            if (!parseServer(server))
                return false;
            if (!expectCloseBrace())
                return false;
            config.addServer(server);
        }
        else{
            printError("Expected 'server' directive");
            return false;
        }
    }
    return true;
}

bool    ConfigParser::parseServer(ServerConfig& server){

}

bool    ConfigParser::parseLocation(LocationConfig& location){
    while (currentToken().type != TOKEN_RBRACE && currentToken().type != TOKEN_EOF){
        
    }
}

bool    ConfigParser::parseServerDirective(ServerConfig& server){

}

bool    ConfigParser::parseLocationDirective(LocationConfig& location){

}