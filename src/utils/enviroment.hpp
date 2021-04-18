/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/enviroment.hpp
*
*****************************************************************************/

#ifndef HI_UTILS_ENVIROMENT_HPP
#define HI_UTILS_ENVIROMENT_HPP

#include <string>

namespace hi
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
} // namespace hi

#endif
