#ifndef CONFIG_PARSER_HPP
# define CONFIG_PARSER_HPP

# include "config.hpp"

# include <string>
# include <vector>

class ConfigParser {
    private:
        std::string _lastError;

        static std::string trim(const std::string& value);
        static std::vector<std::string> splitWords(const std::string& value);
        static bool isUnsignedNumber(const std::string& value);

        bool parseConfigText(const std::string& content, Config& config);
        bool parseServerBlock(const std::string& block, ServerConfig& server);
        bool parseLocationBlock(const std::string& path, const std::string& block, LocationConfig& location);
        bool parseServerDirectory(const std::string& directory, ServerConfig& server);
        bool parseLocationDirectory(const std::string& directory, LocationConfig& location);
        bool parseSizeValue(const std::string& value, size_t& result);
        bool parsePort(const std::string& value, int& port);
        bool parseIndexList(const std::vector<std::string>& words, std::vector<std::string>& target);
        void setError(const std::string& message);

    public:
        ConfigParser();
        ~ConfigParser();

        bool parseFile(const std::string& filename, Config& config);
        bool parseString(const std::string& configContent, Config& config);
        std::string getLastError() const;
};

#endif