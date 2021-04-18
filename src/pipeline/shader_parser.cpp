/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/shader_parser.cpp
*
*****************************************************************************/

#include "pipeline/shader_parser.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_set>

using namespace hi;
using namespace hi::pipeline;

namespace helper
{
size_t findFirstNonwhitespaceCharacter(const std::string_view& str)
{
    auto result = std::find_if(str.begin(), str.end(), [](char c) -> bool { return !std::isspace(c); });
    if (result == str.end())
        return std::string::npos;
    return (result - str.begin());
}
size_t findEndOfToken(const std::string_view& str)
{
    if (str.size() == 0)
        return 0;

    // is single-character token?
    switch (str[0])
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '{':
    case '}':
    case '(':
    case ')':
    case ';':
    case '.':
        return 1;
    }

    // Distinguish = and ==
    if (str[0] == '=')
    {
        return (str[1] == '=') ? 2 : 1;
    }

    enum TokenType
    {
        IDENTIFIER, // [a-zA-Z0-9]
        LITERAL, // [0-9]\+(.[0-9]*(f)?)?
        UNKNOWN
    } state;

    const auto& firstCharacter = str[0];
    if (std::isalpha(firstCharacter))
    {
        state = IDENTIFIER;
    }
    else if (std::isdigit(firstCharacter))
    {
        state = LITERAL;
    }
    else
    {
        state = UNKNOWN;
    }

    size_t endPosition = str.size();
    for (auto it = str.begin() + 1; it < str.end(); it++)
    {
        if (std::isspace(*it))
            return it - str.begin();
        switch (state)
        {
        case IDENTIFIER:
            if (*it == '_')
                continue;
            else if (std::isalpha(*it))
                continue;
            else if (std::isdigit(*it))
                continue;
            else
                return it - str.begin();
            break;
        case LITERAL:
            if (*it != '.' && *it != 'f' && !std::isdigit(*it))
                return it - str.begin();
            break;
        case UNKNOWN:
            return 1;
        }
    }
    return endPosition;
}
std::vector<std::string_view> whitespaceSeparatedTokens(const std::string_view& code)
{
    std::vector<std::string_view> result;
    auto remainingString = code;
    auto startingPosition = 0;
    while ((startingPosition = helper::findFirstNonwhitespaceCharacter(remainingString))
        != std::string::npos)
    {
        auto end = findEndOfToken(remainingString.substr(startingPosition));
        if (end == 0)
            break;
        auto token = remainingString.substr(startingPosition, end);
        result.push_back(token);
        remainingString = remainingString.substr(startingPosition + end);
    }
    return result;
}

} //namespace helper

std::vector<std::string_view> hi::pipeline::tokenize(const std::string_view& code)
{
    return helper::whitespaceSeparatedTokens(code);
}

bool hi::pipeline::isBuiltinGLSLType(const std::string_view& token)
{
    std::string key = std::string(token);
    static const auto builtinTypes = std::unordered_set<std::string> { "vec3", "vec4", "mat3", "mat4", "float", "double" };
    return (builtinTypes.count(key) > 0);
}
