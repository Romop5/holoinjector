#include "utils/enviroment.hpp"

float ve::enviroment::getEnviromentValue(const std::string& variable, float defaultValue)
{
    auto envStringRaw = getenv(variable.c_str());
    if(!envStringRaw)
        return defaultValue;
    float resultValue = defaultValue;
    try {
        resultValue = std::stof(envStringRaw);
    } catch(...) {};
    printf("[Enhancer] Getting env value of %s => %f\n", variable.c_str(),resultValue);
    return resultValue;
}

std::string ve::enviroment::getEnviromentValueStr(const std::string& variable, std::string defaultValue)
{
    auto envStringRaw = getenv(variable.c_str());
    auto result = (envStringRaw)?envStringRaw:defaultValue;
    printf("[Enhancer] Getting env value of %s => %s\n", variable.c_str(),result.c_str());
    return result;
}

void ve::enviroment::getEnviroment(const std::string& variable, float& storage)
{
    storage = getEnviromentValue(variable, storage);
}