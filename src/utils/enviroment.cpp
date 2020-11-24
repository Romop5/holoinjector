#include "utils/enviroment.hpp"
#include "logger.hpp"

bool ve::enviroment::hasEnviromentalVariable(const std::string& variable)
{
    return getenv(variable.c_str()) != nullptr;
}

float ve::enviroment::getEnviromentValue(const std::string& variable, float defaultValue)
{
    auto envStringRaw = getenv(variable.c_str());
    if(!envStringRaw)
        return defaultValue;
    float resultValue = defaultValue;
    try {
        resultValue = std::stof(envStringRaw);
    } catch(...) {};
    Logger::log("[Enhancer] Getting env value of {} => {}\n", variable.c_str(),resultValue);
    return resultValue;
}

std::string ve::enviroment::getEnviromentValueStr(const std::string& variable, std::string defaultValue)
{
    auto envStringRaw = getenv(variable.c_str());
    auto result = (envStringRaw)?envStringRaw:defaultValue;
    Logger::log("[Enhancer] Getting env value of {} => {}\n", variable.c_str(),result.c_str());
    return result;
}

void ve::enviroment::getEnviroment(const std::string& variable, float& storage)
{
    storage = getEnviromentValue(variable, storage);
}
