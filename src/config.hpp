#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <unordered_map>
namespace ve
{
    /**
     * @brief Handles user's config files & enviromental variables
     *
     * Global settings are handled in two ways
     * - config file: settings are stored in text files, at different levels (system-, user-wide)
     * - enviromental variables: these have higher priority
     */
    class Config
    {
        public:

        /**
         * @brief Internal: utility wrapper over dict
         */
        class ConfigDict: public std::unordered_map<std::string, std::string>
        {
            public:
            bool hasKey(const std::string& key) const;
            size_t getAsSizet(const std::string& key) const;
            float getAsFloat(const std::string& key) const;
            const std::string& getAsString(const std::string& key) const;

            /// Serialize config to human-readable form
            const std::string toString() const;
        };
        const ConfigDict load();
        private:
        const std::string getConfigPath() const;
        void loadFromFile(ConfigDict& dict, const std::string& fileName);
        void loadFromEnvironment(ConfigDict& dict);
    };
} // namespace ve
#endif


