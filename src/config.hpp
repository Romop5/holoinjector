#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <unordered_map>
namespace ve
{
    class Context;
    class Config
    {
        public:
        class ConfigDict: public std::unordered_map<std::string, std::string>
        {
            public:
            bool hasKey(const std::string& key) const
            {
                return count(key) > 0;
            }
            size_t getAsSizet(const std::string& key) const
            {
                const auto& value = at(key);
                return std::stol(value);
            }
            float getAsFloat(const std::string& key) const
            {
                const auto& value = at(key);
                return std::stof(value);
            }
            const std::string& getAsString(const std::string& key) const
            {
                return at(key);
            }
        };
        const ConfigDict load();
        private:
        const std::string getConfigPath() const;
        void loadFromFile(ConfigDict& dict, const std::string& fileName);
        void loadFromEnvironment(ConfigDict& dict);
    };
} // namespace ve
#endif


