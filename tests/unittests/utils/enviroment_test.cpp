#include "gtest/gtest.h"
#include "utils/enviroment.hpp"
#include <cstdlib>

using namespace ve;
using namespace ve::enviroment;

namespace {
TEST(Enviroment, hasEnviromentalVariable) {
    unsetenv("env-unittest");
    EXPECT_FALSE(hasEnviromentalVariable("env-unittest"));
    setenv("env-unittest","value",1);
    EXPECT_TRUE(hasEnviromentalVariable("env-unittest"));
    unsetenv("env-unittest");
}

TEST(Enviroment, getEnviromentValue) {
    unsetenv("env-unittest");
    EXPECT_EQ(getEnviromentValue("env-unittest",0.0f), 0.0f);
    setenv("env-unittest",std::to_string(1.0).c_str(),1);
    EXPECT_EQ(getEnviromentValue("env-unittest",0.0f), 1.0f);
    unsetenv("env-unittest");
    EXPECT_EQ(getEnviromentValue("env-unittest",0.0f), 0.0f);
}

TEST(Enviroment, getEnviromentValueStr) {
    unsetenv("env-unittest");
    EXPECT_EQ(getEnviromentValueStr("env-unittest",""), "");
    setenv("env-unittest",std::to_string(1.0).c_str(),1);
    EXPECT_EQ(getEnviromentValue("env-unittest",0.0f), 1.0f);
    unsetenv("env-unittest");
    EXPECT_EQ(getEnviromentValue("env-unittest",0.0f), 0.0f);
}

TEST(Enviroment, getEnviroment) {
    float value;
    setenv("env-unittest",std::to_string(1.0).c_str(),1);
    EXPECT_EQ((getEnviroment("env-unittest", value),value), 1.0);
    setenv("env-unittest",std::to_string(2.0).c_str(),1);
    EXPECT_EQ((getEnviroment("env-unittest", value),value), 2.0);
    setenv("env-unittest",std::to_string(4.0).c_str(),1);
    EXPECT_EQ((getEnviroment("env-unittest", value),value), 4.0);
    setenv("env-unittest",std::to_string(666.0).c_str(),1);
    EXPECT_EQ((getEnviroment("env-unittest", value),value), 666.0);
}
}
