/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/enviroment.cpp
*
*****************************************************************************/

#include "utils/enviroment.hpp"
#include "logger.hpp"

bool hi::enviroment::hasEnviromentalVariable(const std::string& variable)
{
    return getenv(variable.c_str()) != nullptr;
}

float hi::enviroment::getEnviromentValue(const std::string& variable, float defaultValue)
{
    auto envStringRaw = getenv(variable.c_str());
    if (!envStringRaw)
        return defaultValue;
    float resultValue = defaultValue;
    try
    {
        resultValue = std::stof(envStringRaw);
    }
    catch (...)
    {
    };
    Logger::log("[Injector] Getting env value of", variable.c_str(), "=>", resultValue);
    return resultValue;
}

std::string hi::enviroment::getEnviromentValueStr(const std::string& variable, std::string defaultValue)
{
    auto envStringRaw = getenv(variable.c_str());
    auto result = (envStringRaw) ? envStringRaw : defaultValue;
    Logger::log("[Injector] Getting env value of", variable.c_str(), "=>", result.c_str());
    return result;
}

void hi::enviroment::getEnviroment(const std::string& variable, float& storage)
{
    storage = getEnviromentValue(variable, storage);
}
