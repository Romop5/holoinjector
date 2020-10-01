#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <cassert>
#include <cctype>

namespace helper
{
    size_t findFirstNonwhitespaceCharacter(const std::string_view& str)
    {
        auto result = std::find_if(str.begin(),str.end(), [](char c)->bool{ return !std::isspace(c);} );
        if(result == str.end())
            return std::string::npos;
        return (result-str.begin());
    }
    size_t findEndOfToken(const std::string_view& str)
    {
        if(str.size() == 0)
            return 0;

        // is single-character token?
        switch(str[0])
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
        if(str[0] == '=')
        {
            return (str[1] == '=')?2:1;
        }

        enum TokenType
        {
            IDENTIFIER, // [a-zA-Z0-9]
            LITERAL, // [0-9]\+(.[0-9]*(f)?)?
            UNKNOWN
        } state;

        const auto& firstCharacter = str[0];
        if(std::isalpha(firstCharacter))
        {
            state = IDENTIFIER;
        } else if(std::isdigit(firstCharacter))
        {
            state = LITERAL;
        } else {
            state = UNKNOWN;
        }

        size_t endPosition = str.size();
        for(auto it = str.begin()+1;  it < str.end(); it++)
        {
            if(std::isspace(*it))
                return it-str.begin();
            switch(state)
            {
                case IDENTIFIER:
                    if(*it == '_')
                        continue;
                    else if (std::isalpha(*it))
                        continue;
                    else
                        return it-str.begin();
                    break;
                case LITERAL:
                    if(*it != '.' && *it != 'f' && !std::isdigit(*it))
                        return it-str.begin();
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
        while((startingPosition = helper::findFirstNonwhitespaceCharacter(remainingString))
                != std::string::npos)
        {
            auto end = findEndOfToken(remainingString.substr(startingPosition));
            if(end == 0)
                break;
            auto token = remainingString.substr(startingPosition,end);
            result.push_back(token);
            remainingString = remainingString.substr(startingPosition+end);
        }
        return result;
    }

} //namespace helper


int main()
{
    std::cout << "Hello" << std::endl;
    auto tokens = helper::whitespaceSeparatedTokens("gl_Position = 10.0;");
    for(const auto& token: tokens)
    {
        std::cout << "Token: '" << token << "'" << std::endl;
    }
    return 0;
}
