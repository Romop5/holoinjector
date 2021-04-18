#include "gtest/gtest.h"
#include "utils/glsl_preprocess.hpp"

using namespace hi;
using namespace hi::glsl_preprocess;

namespace {
TEST(glsl_preprocess, preprocessGLSLCodeBasic) {
    std::string basicShader = "void main ( ) { }";
    // Basic shader above does not contain any empty lines nor comments
    EXPECT_EQ(preprocessGLSLCode(basicShader), basicShader);

    std::string basicShaderVersion = "#version 330 core\nvoid main() {}";
    // Preprocessing should keep version intact
    auto processedCode = preprocessGLSLCode(basicShaderVersion);
    EXPECT_TRUE(processedCode.find("#version 330 core") != std::string::npos);
            
}

TEST(glsl_preprocess, testMacroExpansion) {
    std::string basicShader = "#ifdef HELLO_WORLD\n int main() {} #endif";
    auto result = preprocessGLSLCode(basicShader);
    // As no HELLO_WORLD is defined, main() should be effictively removed
    EXPECT_TRUE(result.find("main") == std::string::npos);
    // Also macro itself should be removed / expanded
    EXPECT_TRUE(result.find("ifdef") == std::string::npos);
    EXPECT_TRUE(result.find("HELLO_WORLD") == std::string::npos);
}

TEST(glsl_preprocess, joinShaders) {
    std::string aShader = "int a() {}";
    std::string bShader = "int b() {}";
    std::vector<char*> shaderList = {aShader.data(), bShader.data()};
    auto result = joinGLSLshaders(2, shaderList.data(), 0);
}
TEST(glsl_preprocess, removeComments) {
    std::string basicShader = "// secrete comment to be removed\nthis stays in";
    auto result = removeComments(basicShader);
    EXPECT_TRUE(result.find("secrete") == std::string::npos);
    EXPECT_TRUE(result.find("comment") == std::string::npos);
    EXPECT_TRUE(result.find("removed") == std::string::npos);
    EXPECT_FALSE(result.find("this") == std::string::npos);
    EXPECT_FALSE(result.find("stays") == std::string::npos);
}
}
