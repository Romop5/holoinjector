#include <regex>
#include "simplecpp.h" // CPP preprocessor

#include "utils/glsl_preprocess.hpp"

std::string ve::glsl_preprocess::wrapGLSLMacros(std::string code)
{
    auto regV = std::regex("#version");
    auto result = std::regex_replace (code,regV,"GLSL_VERSION",std::regex_constants::match_default);

    auto regE = std::regex("#extension");
    result = std::regex_replace (result,regE,"GLSL_EXTENSION",std::regex_constants::match_default);
    return result;
}
std::string ve::glsl_preprocess::unwrapGLSLMacros(std::string code)
{
    auto regV = std::regex("GLSL_VERSION");
    auto result = std::regex_replace (code,regV,"#version",std::regex_constants::match_default);

    auto regE = std::regex("GLSL_EXTENSION");
    result = std::regex_replace (result,regE,"#extension",std::regex_constants::match_default);
    return result;
}
std::string ve::glsl_preprocess::preprocessGLSLCode(std::string code)
{
    std::stringstream ss;
    ss << wrapGLSLMacros(code);
    auto tmp = unwrapGLSLMacros(simplecpp::preprocess_inmemory(ss));
    return std::regex_replace(tmp, std::regex("^$"),"");
}

std::string ve::glsl_preprocess::joinGLSLshaders(GLsizei count, const GLchar* const*string, const GLint* length)
{
    std::stringstream ss;
    for(size_t i = 0; i < count; i++)
    {
        std::string_view file = length?std::string_view(string[i], length[i]):string[i];
        ss << file << "\n";
    }
    return ss.str();
}

