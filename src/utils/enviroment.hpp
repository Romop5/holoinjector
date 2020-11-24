#ifndef VE_UTILS_ENVIROMENT_HPP
#define VE_UTILS_ENVIROMENT_HPP

#include <string>

namespace ve
{
namespace enviroment
{
    bool hasEnviromentalVariable(const std::string& variable);
    /// Get value of enviromental variable as float
    float getEnviromentValue(const std::string& variable, float defaultValue = 0.0f);

    /// Get value of enviromental variable as string
    std::string getEnviromentValueStr(const std::string& variable, std::string defaultValue = "");

    /// Store value of variable to 'storage' (or store 0.0)
    void getEnviroment(const std::string& variable, float& storage);
};
} // namespace ve

#endif
