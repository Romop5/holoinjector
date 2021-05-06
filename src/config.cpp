/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        config.cpp
*
*****************************************************************************/

#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "config.hpp"
#include "context.hpp"
#include "logger.hpp"
#include "utils/enviroment.hpp"

using namespace hi;

///////////////////////////////////////////////////////////////////////////////
// ConfigDict
///////////////////////////////////////////////////////////////////////////////
bool Config::ConfigDict::hasKey(const std::string& key) const
{
    return count(key) > 0;
}
size_t Config::ConfigDict::getAsSizet(const std::string& key) const
{
    const auto& value = at(key);
    return std::stol(value);
}
float Config::ConfigDict::getAsFloat(const std::string& key) const
{
    const auto& value = at(key);
    return std::stof(value);
}
const std::string& Config::ConfigDict::getAsString(const std::string& key) const
{
    return at(key);
}

const std::string Config::ConfigDict::toString() const
{
    std::stringstream ss;
    for (const auto& [key, value] : *this)
    {
        ss << key << " [ " << value << " ], ";
    }
    return ss.str();
}
///////////////////////////////////////////////////////////////////////////////
// Config
///////////////////////////////////////////////////////////////////////////////
const Config::ConfigDict Config::load()
{
    ConfigDict dict;
    const auto configFile = getConfigPath();
    Logger::log("Loading config from: ", configFile);
    if (!configFile.empty())
    {
        loadFromFile(dict, configFile);
    }
    loadFromEnvironment(dict);
    return dict;
}

const std::string Config::getConfigPath() const
{
    // 1. has enviroment variable HI_CONFIG
    if (enviroment::hasEnviromentalVariable("HI_CONFIG"))
    {
        return enviroment::getEnviromentValueStr("HI_CONFIG", "");
    }
    // 2. has holoinjector.cfg in current work dir?
    const auto relativeConfig = std::filesystem::current_path() / "holoinjector.cfg";
    if (std::filesystem::exists(relativeConfig))
    {
        return relativeConfig;
    }
    // 3. has holoinjector.cfg in ~/.config/holoinjector/
    const auto homeDir = enviroment::getEnviromentValueStr("HOME", "/tmp");
    const auto relativeHomeConfig = std::filesystem::path(homeDir) / ".config/holoinjector/holoinjector.cfg";
    if (std::filesystem::exists(relativeHomeConfig))
    {
        return relativeHomeConfig;
    }
    // 4. search for system-wide config
    if (std::filesystem::exists("/etc/holoinjector.cfg"))
    {
        return "/etc/holoinjector.cfg";
    }
    // if no config file was found, simply return an empty string
    return "";
}

void Config::loadFromFile(ConfigDict& dict, const std::string& fileName)
{
    static std::vector<std::pair<std::string, std::string>> keyDefaultValues = {
        { "outputXSize", "512" },
        { "outputYSize", "512" },
        { "gridXSize", "5" },
        { "gridYSize", "9" },
    };

    YAML::Node config = YAML::LoadFile(fileName);
    for (const auto& entry : keyDefaultValues)
    {
        const auto& key = entry.first;
        const auto& defaultValue = entry.second;
        if (config[key])
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
    static std::vector<std::pair<std::string, std::string>> enviromentVariables = {
        { "HI_XMULTIPLIER", "xmultiplier" },
        { "HI_DISTANCE", "distance" },
        { "HI_NOW", "now" },
        { "HI_QUILT", "quilt" },
        { "HI_WIDE", "wide" },
        { "HI_QUILTX", "gridXSize" },
        { "HI_QUILTY", "gridYSize" },
        { "HI_FBOWIDTH", "outputXSize" },
        { "HI_FBOHEIGHT", "outputYSize" },
        { "HI_EXIT_AFTER", "exitAfterFrames" },
        { "HI_CAMERAID", "onlyShownCameraID" },
        { "HI_SCREENSHOT", "screenshotFormatString" },
        { "HI_NONINTRUSIVE", "shouldBeNonIntrusive" },
        { "HI_RUNINBG", "runInBackground" },
        { "HI_RECORDFPS", "shouldRecordFPS" },
        { "HI_VERTEX", "noGeometryShader" },
    };
    for (const auto& entry : enviromentVariables)
    {
        const auto& enviromentName = entry.first;
        const auto& keyName = entry.second;
        if (enviroment::hasEnviromentalVariable(enviromentName))
            dict[keyName] = enviroment::getEnviromentValueStr(enviromentName);
    }
}
