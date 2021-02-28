#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP
#include <string>
#include <regex>
#include <functional>

namespace ve
{
namespace utils
{
    std::string regex_replace_functor(const std::string str, const std::regex reg, std::function<std::string(std::string)> functor);

    std::string regex_replace_identifiers(const std::string str, const std::string identifierName, std::function<std::string()> functor);
}
}

#endif
