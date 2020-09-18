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

    /// Verifies if 'token' names a GLSL builtin type
    bool isBuiltinGLSLType(const std::string& token);

    /// Get string repr. from declaration or empty string if variable is not declared
    std::string getVariableType(const std::string& variable, const std::string& sourceCode);

    /// Verifies if token 'identifier' is declared as uniform variable in source code
    bool isUniformVariable(const std::string& identifier, const std::string& sourceCode);

    /// Finds all assigment to gl_Position, and returns text string, representing such statment
    std::vector<VertextAssignment> findAllOutVertexAssignments(const std::string& sourceCode);

    /// Injects Enhancer-specific transformation into shader
    std::string injectShader(const std::vector<VertextAssignment>& assignments, const std::string& sourceCode);
} //namespace ve

#endif
