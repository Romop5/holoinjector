#include <vector>
#include <filesystem>
#include <fstream>
#include "yaml-cpp/yaml.h"

#include "config.hpp"
#include "context.hpp"
#include "utils/enviroment.hpp"
#include "logger.hpp"

using namespace ve;
const Config::ConfigDict Config::load()
{
    ConfigDict dict;
    const auto configFile = getConfigPath();
    Logger::log("Loading config from: {}\n", configFile);
    if(!configFile.empty())
    {
        loadFromFile(dict,configFile);
    }
    loadFromEnvironment(dict);
    return dict;
}

const std::string Config::getConfigPath() const
{
    // 1. has enviroment variable ENHANCER_CONFIG
    if(enviroment::hasEnviromentalVariable("EHANCER_CONFIG"))
    {
        return enviroment::getEnviromentValueStr("ENHANCER_CONFIG", "");
    }
    // 2. has enhancer.cfg in current work dir?
    const auto relativeConfig = std::filesystem::current_path() / "enhancer.cfg";
    if(std::filesystem::exists(relativeConfig))
    {
        return relativeConfig;
    }
    // 3. has enhancer.cfg in ~/.config/enhancer/
    const auto homeDir = enviroment::getEnviromentValueStr("HOME", "/tmp");
    const auto relativeHomeConfig = std::filesystem::path(homeDir) / ".config/enhancer/enhancer.cfg";
    if(std::filesystem::exists(relativeHomeConfig))
    {
        return relativeHomeConfig;
    }
    // 4. search for system-wide config
    if(std::filesystem::exists("/etc/enhancer.cfg"))
    {
        return "/etc/enhancer.cfg";
    }
    // if no config file was found, simply return an empty string
    return "";
}
void Config::loadFromFile(ConfigDict& dict,const std::string& fileName)
{
    static std::vector<std::pair<std::string, std::string>> keyDefaultValues =
    {
        {"outputXSize", "512"},
        {"outputYSize", "512"},
    };
    
    YAML::Node config = YAML::LoadFile(fileName);
    for(const auto& entry: keyDefaultValues)
    {
        const auto& key = entry.first;
        const auto& defaultValue = entry.second;
        if(config[key])
            dict[key] = config[key].as<std::string>();
        else
        {
            config[key] = defaultValue;
            dict[key] = defaultValue;
        }
    }
    // Store default values
    std::ofstream fout(fileName);
    fout << config;
}
void Config::loadFromEnvironment(ConfigDict& dict)
{
    static std::vector<std::pair<std::string, std::string>> enviromentVariables =
    {
        {"ENHANCER_XMULTIPLIER", "xmultiplier"},
        {"ENHANCER_DISTANCE", "distance"},
        {"ENHANCER_NOW", "now"},
        {"ENHANCER_EXIT_AFTER", "exitAfterFrames"},
        {"ENHANCER_CAMERAID", "onlyShownCameraID"},
        {"ENHANCER_SCREENSHOT", "screenshotFormatString"},
        {"ENHANCER_NONINTRUSIVE", "shouldBeNonIntrusive"},
    };
    for(const auto& entry: enviromentVariables)
    {
        const auto& enviromentName = entry.first;
        const auto& keyName = entry.second;
        if(enviroment::hasEnviromentalVariable(enviromentName))
            dict[keyName] = enviroment::getEnviromentValueStr(enviromentName);
    }
}
