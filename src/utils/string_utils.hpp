/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/string_utils.hpp
*
*****************************************************************************/

#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP
#include <functional>
#include <regex>
#include <string>

namespace hi
{
namespace utils
{
    std::string regex_replace_functor(const std::string str, const std::regex reg, std::function<std::string(std::string)> functor);

    std::string regex_replace_identifiers(const std::string str, const std::string identifierName, std::function<std::string()> functor);
}
}

#endif
