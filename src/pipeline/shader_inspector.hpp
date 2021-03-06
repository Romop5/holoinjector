/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/shader_inspector.hpp
*
*****************************************************************************/

#ifndef HI_SHADER_INSPECTOR_HPP
#define HI_SHADER_INSPECTOR_HPP

#include <string>
#include <vector>

namespace hi
{
namespace pipeline
{
    class ShaderInspector
    {
    protected:
        std::string sourceCode;

    public:
        ShaderInspector(std::string code)
            : sourceCode(std::move(code))
        {
        }

        enum AnalysisType
        {
            UNIFORM,
            INPUT,
            FUNCTION,
            GLPOSITION,
            POSSIBLE_TEMPORARY_VARIABLE,
            CONSTANT_ASSIGNMENT
        };
        struct Analysis
        {
            std::string foundIdentifier;
            AnalysisType type;
        };

        struct VertextAssignment
        {
            bool isFixedPipelineUsed = false;
            size_t positionInCode;
            std::string statementRawText;
            std::string transformName;
            Analysis analysis;
        };

        /// Get string repr. from declaration or empty string if variable is not declared
        std::string getVariableType(const std::string& variable) const;

        bool isIdentifier(const std::string_view& token) const;

        /// Verifies if identifier is an uniform in any interface block
        bool isUniformVariableInInterfaceBlock(const std::string& identifier) const;

        /// Get block name
        std::string getUniformBlockName(const std::string& uniformName) const;

        /// Verifies if token 'identifier' is declared as uniform variable in source code
        bool isUniformVariable(const std::string& identifier) const;

        /// Finds all assigment to gl_Position, and returns text string, representing such statment
        std::vector<VertextAssignment> findAllOutVertexAssignments() const;

        /// Compute final statement
        std::string replaceGLAssignement(VertextAssignment originalStatement);

        /// Injects Injector-specific transformation into shader and returns modified code
        std::string injectShader(const std::vector<VertextAssignment>& assignments);

        /// Get transformation matrix from assigments
        std::string getTransformationUniformName(std::vector<VertextAssignment>);

        /// Get count of declared uniforms in shader in O(n)
        size_t getCountOfUniforms() const;

        /// List uniforms pairs <type, name>
        using TypeNamePairList = std::vector<std::pair<std::string, std::string>>;
        TypeNamePairList getListOfUniforms() const;
        TypeNamePairList getListOfInputs() const;
        TypeNamePairList getListOfOutputs() const;
        TypeNamePairList getListOfVaryings() const;
        TypeNamePairList mergeList(const TypeNamePairList a, const TypeNamePairList b) const;

        Analysis analyzeGLPositionAssignment(std::string& assignment) const;
        std::string replaceGLPositionAssignment(VertextAssignment assignment) const;

        // Attempts to choose one of uniforms based on heuristic
        std::string getFallbackUniformViaHeuristic() const;

        std::string recursivelySearchUniformFromTemporaryVariable(std::string tmpName, size_t remainingDepth = 10) const;

        /// Is clip-space shader
        bool isClipSpaceShader() const;
        bool hasFtransform() const;

        static void injectCommonCode(std::string& sourceOriginal);

        //TODO: separate into logic module
        static std::string getCommonTransformationShader();
    };
} //namespace pipeline
} //namespace hi

#endif
