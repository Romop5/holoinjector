/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/string_utils.cpp
*
*****************************************************************************/

#include "utils/string_utils.hpp"
#include <functional>

using namespace hi::utils;

/// Iterate&replace over generic text, matched by regex
std::string hi::utils::regex_replace_functor(const std::string str, const std::regex reg, std::function<std::string(std::string)> functor)
{
    std::string result = str;
    size_t startPosition = 0;
    size_t currentLength = result.size();
    auto end = std::sregex_iterator();
    do
    {
        auto it = std::sregex_iterator(result.begin() + startPosition, result.end(), reg);
        if (it == end)
            break;
        auto& match = *it;
        auto newString = functor(match.str(0));
        // replace original with new
        result.replace(startPosition + match.position(0), match.str(0).size(), newString);
        // advance pass the replaced string
        startPosition += match.position(0) + newString.size();
        currentLength = result.size();
    } while (startPosition < currentLength);
    return result;
}

/// Iterate&replace over all identifiers in text
std::string hi::utils::regex_replace_identifiers(const std::string str, const std::string identifierName, std::function<std::string()> functor)
{
    auto replaceRegexSearch = std::regex(std::string("([a-zA-Z_-][a-zA-Z0-9_-]*)?") + identifierName + std::string("[a-zA-Z0-9_-]*"));
    return hi::utils::regex_replace_functor(str, replaceRegexSearch, [&](auto str) -> std::string {
        if (str == identifierName)
            return functor();
        return str;
    });
}


size_t hi::utils::computeHash(const std::string str)
{
    // Note: std::hash is implementation-dependent -> this could be replacedwith SHA1/MD5 instead 
    // See note in https://en.cppreference.com/w/cpp/utility/hash
    return std::hash<std::string>{}(str);
}
