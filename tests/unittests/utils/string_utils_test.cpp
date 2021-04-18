#include "gtest/gtest.h"
#include "utils/string_utils.hpp"
#include <cstdlib>

using namespace hi;
using namespace hi::utils;

namespace {
TEST(StringUtils, Simple) {
    std::string str = "aa";

    auto newStr = regex_replace_functor(str,std::regex("a"),[](auto s)
    {
        return "bb";
    });
    ASSERT_EQ(newStr, "bbbb");
}
TEST(StringUtils, BasicI) {
    std::string str = "roman man";

    auto newStr = regex_replace_functor(str,std::regex("an"),[](auto s)
    {
        return "en man";
    });
    ASSERT_EQ(newStr, "romen man men man");
}
TEST(StringUtils, BasicII) {
    std::string str = "roman man an";

    auto newStr = regex_replace_functor(str,std::regex("an"),[](auto s)
    {
        return "en man";
    });
    ASSERT_EQ(newStr, "romen man men man en man");
}
}
