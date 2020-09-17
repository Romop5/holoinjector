#ifndef VE_SHADER_INSPECTOR_HPP
#define VE_SHADER_INSPECTOR_HPP

#include <string>
#include <vector>

namespace ve
{
    struct VertextAssignment
    {
        std::string statementRawText;
        std::string firstTokenFromLeft;
    };

    bool isBuiltinGLSLType(const std::string& token);
    std::string getVariableType(const std::string& variable, const std::string& sourceCode);
    bool isUniformVariable(const std::string& identifier, const std::string& sourceCode);

    std::vector<VertextAssignment> findAllOutVertexAssignments(const std::string& sourceCode);
} //namespace ve

#endif
