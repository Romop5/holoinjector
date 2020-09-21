#ifndef VE_SHADER_INSPECTOR_HPP
#define VE_SHADER_INSPECTOR_HPP

#include <string>
#include <vector>

namespace ve
{
    class ShaderInspector
    {
        protected:
        std::string sourceCode;
        public:
        ShaderInspector(std::string code): sourceCode(std::move(code)) {}

        struct VertextAssignment
        {
            std::string statementRawText;
            std::string firstTokenFromLeft;
        };

        /// Verifies if 'token' names a GLSL builtin type
        static bool isBuiltinGLSLType(const std::string& token);

        /// Get string repr. from declaration or empty string if variable is not declared
        std::string getVariableType(const std::string& variable) const;

        /// Verifies if token 'identifier' is declared as uniform variable in source code
        bool isUniformVariable(const std::string& identifier) const;

        /// Finds all assigment to gl_Position, and returns text string, representing such statment
        std::vector<VertextAssignment> findAllOutVertexAssignments() const;

        /// Injects Enhancer-specific transformation into shader and returns modified code
        std::string injectShader(const std::vector<VertextAssignment>& assignments);

        /// Get transformation matrix from assigments
        std::string getTransformationUniformName(std::vector<VertextAssignment>);

        /// Get count of declared uniforms in shader in O(n)
        size_t getCountOfUniforms() const;
    };
} //namespace ve

#endif
