#include "gtest/gtest.h"
#include "pipeline/shader_parser.hpp"

#include <iostream>
#include <functional>

namespace helper
{
    template<typename C1, typename C2>
    size_t concurrent_for(const C1& c1, const C2& c2, std::function<bool(typename C1::value_type, typename C2::value_type,size_t)> f)
    {
        for(auto it1 = c1.begin(), it2 = c2.begin(); it1 != c1.end() && it2 != c2.end(); it1++, it2++)
        {
            const size_t position = it1-c1.begin();
            if(f(*it1,*it2,position))
                return position;
        }
        return std::min(c1.size(),c2.size());
    }
    using token_stream = std::vector<std::string_view>;
    struct StreamComparison
    {
        bool areSame;
        bool areSameSize;
        size_t indexOfFirstDifference;
    };
    StreamComparison compareStreams(token_stream a, token_stream b)
    {
        StreamComparison result;
        result.areSameSize = a.size() == b.size();
        result.indexOfFirstDifference = concurrent_for(a,b, [] (auto a, auto b, size_t pos) {
            return (a != b);
        });
        result.areSame = result.areSameSize && result.indexOfFirstDifference == a.size();
        return result;
    }
    void dump_tokens(const token_stream& s)
    {
        std::cout << "Found tokens: ";
        for(const auto& token: s)
        {
            std::cout  << "'" << token << "' ";
        }
        std::cout << std::endl;
    }
    void dump_comparison(StreamComparison& cmp, const token_stream& a, const token_stream& b)
    {
        if(!cmp.areSame)
        {
            if(cmp.indexOfFirstDifference < std::min(a.size(), b.size()))
                std::cout << "Tokens differ: " << a[cmp.indexOfFirstDifference] << " vs " << b[cmp.indexOfFirstDifference] << std::endl;
            helper::dump_tokens(a);
        }
    }
} //namespace helper

#define EXPECT_TOKENS(code, ...)\
{\
    auto tokens = ve::pipeline::tokenize(code);\
    helper::token_stream expectedTokens = {__VA_ARGS__ };\
    auto comparison = helper::compareStreams(tokens, expectedTokens);\
    EXPECT_TRUE(comparison.areSame);\
    EXPECT_TRUE(comparison.areSameSize);\
    helper::dump_comparison(comparison, tokens, expectedTokens);\
}

namespace {
TEST(ShaderPArser, Lexing) {
    EXPECT_TOKENS("gl_Position    =   P*   MVP*vec4(1.0);", "gl_Position", "=", "P", "*", "MVP", "*", "vec4",
            "(", "1.0", ")", ";")
    EXPECT_TOKENS("uniform mat4 MVP \t ;", "uniform", "mat4", "MVP", ";")
    EXPECT_TOKENS("vec3 pos = (MVP*vec4(1.0)).xyz;", "vec3","pos","=","(","MVP","*","vec4","(","1.0",")",")",".","xyz",";")
    EXPECT_TOKENS("vec4 pos = vec4(aPos, 1.0);", "vec4","pos","=","vec4","(","aPos",",","1.0",")", ";")
}
}
