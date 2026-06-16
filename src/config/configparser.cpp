#include "configparser.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

std::string ConfigParser::getLastError() const {
    return _lastError;
}

void ConfigParser::setError(const std::string& message) {
    _lastError = message;
}

static std::string skipComments(const std::string& content)
{
    std::stringstream input(content);
    std::stringstream output;
    std::string line;

    while (std::getline(input, line))
    {
        size_t pos = line.find('#');

        if (pos != std::string::npos)
            line.erase(pos);

        output << line << '\n';
    }
    return output.str();
}

std::string ConfigParser::trim(const std::string& value) {
    std::string::size_type begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])))
        ++begin;
    std::string::size_type end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])))
        --end;
    return value.substr(begin, end - begin);
}

std::vector<std::string> ConfigParser::splitWords(const std::string& value) {
    std::vector<std::string> words;
    std::istringstream stream(value);
    std::string word;
    while (stream >> word)
        words.push_back(word);
    return words;
}

bool ConfigParser::isUnsignedNumber(const std::string& value) {
    if (value.empty())
        return false;
    for (std::string::size_type i = 0; i < value.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(value[i])))
            return false;
    }
    return true;
}

bool ConfigParser::parsePort(const std::string& value, int& port) {
    if (!isUnsignedNumber(value))
        return false;
    long parsed = std::strtol(value.c_str(), NULL, 10);
    if (parsed < 1 || parsed > 65535)
        return false;
    port = static_cast<int>(parsed);
    return true;
}

bool ConfigParser::parseErrorCode(const std::string& value, int& code) {
    if (!isUnsignedNumber(value))
        return false;
    long parsed = std::strtol(value.c_str(), NULL, 10);
    if (parsed < 100 || parsed > 599)
        return false;
    code = static_cast<int>(parsed);
    return true;
}

bool ConfigParser::parseSizeValue(const std::string& value, size_t& result) {
    if (value.empty())
        return false;
    std::string number = value;
    size_t multiplier = 1;
    char suffix = static_cast<char>(std::tolower(static_cast<unsigned char>(value[value.size() - 1])));
    if (suffix == 'k' || suffix == 'm' || suffix == 'g') {
        number = value.substr(0, value.size() - 1);
        if (suffix == 'k')
            multiplier = 1024;
        else if (suffix == 'm')
            multiplier = 1024 * 1024;
        else
            multiplier = 1024 * 1024 * 1024;
    }
    if (!isUnsignedNumber(number))
        return false;
    result = static_cast<size_t>(std::strtoull(number.c_str(), NULL, 10) * multiplier);
    return true;
}

bool ConfigParser::parseIndexList(const std::vector<std::string>& words, std::vector<std::string>& target) {
    if (words.size() < 2)
        return false;
    target.assign(words.begin() + 1, words.end());
    return true;
}

bool ConfigParser::parseServerDirective(const std::string& directive, ServerConfig& server) {
    std::string line = trim(directive);
    if (line.empty())
        return true;
    if (line[line.size() - 1] != ';') {
        setError("Missing ';' at end of server directive: " + line);
        return false;
    }
    line = trim(line.substr(0, line.size() - 1));
    std::vector<std::string> words = splitWords(line);
    if (words.empty())
        return true;

    if (words[0] == "listen") {
        if (words.size() < 2) {
            setError("listen directive requires a port");
            return false;
        }
        int port = 0;
        if (!parsePort(words[1], port)) {
            setError("Invalid listen port: " + words[1]);
            return false;
        }
        server.addListenPort(port);
        return true;
    }
    if (words[0] == "server_name") {
        if (words.size() < 2) {
            setError("server_name directive requires at least one name");
            return false;
        }
        for (size_t i = 1; i < words.size(); ++i)
            server.addServerName(words[i]);
        return true;
    }
    if (words[0] == "root") {
        if (words.size() != 2) {
            setError("root directive requires exactly one path");
            return false;
        }
        server.root = words[1];
        return true;
    }
    if (words[0] == "index") {
        if (!parseIndexList(words, server.indice)) {
            setError("index directive requires at least one file");
            return false;
        }
        return true;
    }
    if (words[0] == "client_max_body_size") {
        if (words.size() != 2) {
            setError("client_max_body_size requires one value");
            return false;
        }
        size_t sizeValue = 0;
        if (!parseSizeValue(words[1], sizeValue)) {
            setError("Invalid client_max_body_size value: " + words[1]);
            return false;
        }
        server.clientMaxBody = sizeValue;
        return true;
    }
    if (words[0] == "error_page") {
        if (words.size() < 3) {
            setError("error_page requires at least one code and one path");
            return false;
        }
        std::string page = words.back();
        for (size_t i = 1; i + 1 < words.size(); i++) {
            int code;
            if (!parseErrorCode(words[i], code)) {
                setError("Invalid error page code: " + words[i]);
                return false;
            }
            server.addErrorPage(code, words[i + 1]);
        }
        return true;
    }

    setError("Unknown server directive: " + words[0]);
    return false;
}

bool ConfigParser::parseLocationDirective(const std::string& directive, LocationConfig& location) {
    std::string line = trim(directive);
    if (line.empty())
        return true;
    if (line[line.size() - 1] != ';') {
        setError("Missing ';' at end of location directive: " + line);
        return false;
    }
    line = trim(line.substr(0, line.size() - 1));
    std::vector<std::string> words = splitWords(line);
    if (words.empty())
        return true;

    if (words[0] == "root") {
        if (words.size() != 2) {
            setError("root directive requires exactly one path");
            return false;
        }
        location.root = words[1];
        return true;
    }
    if (words[0] == "alias") {
        if (words.size() != 2) {
            setError("alias directive requires exactly one path");
            return false;
        }
        location.alias = words[1];
        return true;
    }
    if (words[0] == "index")
        return parseIndexList(words, location.index);
    if (words[0] == "allow_methods") {
        if (words.size() < 2) {
            setError("allow_methods requires at least one method");
            return false;
        }
        location.allowMethods.assign(words.begin() + 1, words.end());
        return true;
    }
    if (words[0] == "autoindex") {
        if (words.size() != 2) {
            setError("autoindex requires on or off");
            return false;
        }
        if (words[1] == "on")
            location.autoindex = true;
        else if (words[1] == "off")
            location.autoindex = false;
        else {
            setError("autoindex accepts only on or off");
            return false;
        }
        return true;
    }
    if (words[0] == "client_max_body_size") {
        if (words.size() != 2) {
            setError("client_max_body_size requires one value");
            return false;
        }
        size_t sizeValue = 0;
        if (!parseSizeValue(words[1], sizeValue)) {
            setError("Invalid client_max_body_size value: " + words[1]);
            return false;
        }
        location.clientMaxBody = sizeValue;
        return true;
    }
    if (words[0] == "cgi_pass"){
        if (words.size() != 2){
            setError("cgi_pass requires a path");
            return false;
        }
        location.cgiPass = words[1];
        return true;
    }
    if (words[0] == "cgi"){
        if (words.size() != 3){
            setError("cgi requires extension and executable");
            return false;
        }
        location.cgiHandlers[words[1]] = words[2];
        return true;
    }
    if (words[0] == "return"){
        if (words.size() < 2 || words.size() > 3){
            setError("return requires code [url]");
            return false;
        }
        int code;
        if (!parseErrorCode(words[1], code)){
            setError("invalid return code");
            return false;
        }
        location.hasReturn = true;
        location.returnCode = code;
        if (words.size() == 3)
            location.returnUrl = words[2];
        return true;
    }

    setError("Unknown location directive: " + words[0]);
    return false;
}

bool ConfigParser::parseLocationBlock(const std::string& path, const std::string& block, LocationConfig& location) {
    location = LocationConfig(path);
    std::string::size_type pos = 0;
    while (pos < block.size()) {
        while (pos < block.size() && std::isspace(static_cast<unsigned char>(block[pos])))
            ++pos;
        if (pos >= block.size())
            break;
        std::string::size_type end = block.find(';', pos);
        if (end == std::string::npos) {
            setError("Missing ';' inside location block: " + path);
            return false;
        }
        if (!parseLocationDirective(block.substr(pos, end - pos + 1), location))
            return false;
        pos = end + 1;
    }
    return true;
}

bool ConfigParser::parseServerBlock(const std::string& block, ServerConfig& server) {
    std::string::size_type pos = 0;
    while (pos < block.size()) {
        while (pos < block.size() && std::isspace(static_cast<unsigned char>(block[pos])))
            ++pos;
        if (pos >= block.size())
            break;

        if (block.compare(pos, 8, "location") == 0 && (pos + 8 == block.size() || std::isspace(static_cast<unsigned char>(block[pos + 8])))) {
            pos += 8;
            while (pos < block.size() && std::isspace(static_cast<unsigned char>(block[pos])))
                ++pos;
            std::string::size_type pathEnd = pos;
            while (pathEnd < block.size() && !std::isspace(static_cast<unsigned char>(block[pathEnd])) && block[pathEnd] != '{')
                ++pathEnd;
            std::string locationPath = trim(block.substr(pos, pathEnd - pos));
            if (locationPath.empty()) {
                setError("location directive requires a path");
                return false;
            }
            std::string::size_type braceOpen = block.find('{', pathEnd);
            if (braceOpen == std::string::npos) {
                setError("location block missing '{' for path: " + locationPath);
                return false;
            }
            int depth = 1;
            std::string::size_type bodyStart = braceOpen + 1;
            std::string::size_type bodyEnd = bodyStart;
            for (; bodyEnd < block.size(); ++bodyEnd) {
                if (block[bodyEnd] == '{')
                    ++depth;
                else if (block[bodyEnd] == '}') {
                    --depth;
                    if (depth == 0)
                        break;
                }
            }
            if (depth != 0) {
                setError("location block missing closing '}' for path: " + locationPath);
                return false;
            }
            LocationConfig location;
            if (!parseLocationBlock(locationPath, block.substr(bodyStart, bodyEnd - bodyStart), location))
                return false;
            if (server.hasLocation(location.path)){
                setError("Duplicate location path: " + location.path);
                return false;
            }
            server.addLocation(location);
            pos = bodyEnd + 1;
            continue;
        }

        std::string::size_type end = block.find(';', pos);
        if (end == std::string::npos) {
            setError("Missing ';' inside server block");
            return false;
        }
        if (!parseServerDirective(block.substr(pos, end - pos + 1), server))
            return false;
        pos = end + 1;
    }
    return true;
}

bool ConfigParser::parseConfigText(const std::string& content, Config& config) {
    config.clear();
    std::string::size_type pos = 0;
    while (pos < content.size()) {
        while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos])))
            ++pos;
        if (pos >= content.size())
            break;
        if (content.compare(pos, 6, "server") != 0 || (pos + 6 < content.size() && !std::isspace(static_cast<unsigned char>(content[pos + 6])) && content[pos + 6] != '{')) {
            setError("Expected 'server' block");
            return false;
        }
        pos += 6;
        while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos])))
            ++pos;
        if (pos >= content.size() || content[pos] != '{') {
            setError("Expected '{' after server");
            return false;
        }
        ++pos;
        int depth = 1;
        std::string::size_type bodyStart = pos;
        while (pos < content.size()) {
            if (content[pos] == '{')
                ++depth;
            else if (content[pos] == '}') {
                --depth;
                if (depth == 0)
                    break;
            }
            ++pos;
        }
        if (depth != 0) {
            setError("Unclosed server block");
            return false;
        }

        ServerConfig server;
        if (!parseServerBlock(content.substr(bodyStart, pos - bodyStart), server))
            return false;
        if (server.listen.empty())
            server.addListenPort(80);
        config.addServer(server);
        ++pos;
    }
    return !config.empty();
}

bool ConfigParser::parseFile(const std::string& filename, Config& config) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        setError("Cannot open file: " + filename);
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return parseString(content, config);
}

bool ConfigParser::parseString(const std::string& configContent, Config& config) {
    _lastError.clear();
    std::string clean = skipComments(configContent);
    return parseConfigText(clean, config);
}