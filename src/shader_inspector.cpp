#include "shader_inspector.hpp"
#include <regex>
#include <unordered_set>

#include <iostream>

using namespace ve;

bool ve::isBuiltinGLSLType(const std::string& token)
{
    static const auto builtinTypes = std::unordered_set<std::string>{ "vec3", "vec4", "mat3", "mat4", "float", "double"};
    return (builtinTypes.count(token) > 0);
}

bool ve::isUniformVariable(const std::string& identifier, const std::string& sourceCode)
{
    std::string regexLiteral = std::string("uniform[^;]*[\f\n\r\t\v ]") + identifier + std::string("[\f\n\r\t\v ]*;");
    auto isDefinedAsUniform = std::regex(regexLiteral,std::regex::extended);
    std::smatch m;
    std::regex_search(sourceCode, m, isDefinedAsUniform); 
    return m.size() > 0;
}


std::string ve::getVariableType(const std::string& variable, const std::string& sourceCode)
{
    std::string regexLiteral = std::string("[a-zA-Z0-9]+[\f\n\r\t\v ]+") + variable+ std::string("[\f\n\r\t\v ]*;");
    auto definitionPattern = std::regex(regexLiteral,std::regex::extended);
    std::smatch m;
    std::regex_search(sourceCode, m, definitionPattern); 
    if(!m.size())
        return "";
    auto definitionStatementRawText = std::string(m[0]);
    auto firstWhitespacePosition = definitionStatementRawText.find_first_of("\f\n\r\t\v ");
    return definitionStatementRawText.substr(0, firstWhitespacePosition);
}

namespace helper {
    /**
     * @brief Returns first right-hand identifier/token literal
     * @return "" or token
     */
    std::string getFirstTokenAfterEq(const std::string assignmentStatement)
    {
        // trim gl_Position = X*Y*Z.... to '= X'
        static auto firstTokenPattern = std::regex("=[\f\n\r\t\v ]*[a-zA-Z0-9_]*");
        std::smatch m;
        std::regex_search(assignmentStatement, m, firstTokenPattern); 

        // if trimming was possible 
        if(m.size())
        {
            // then take everything since last whitespace character till the end
            const std::string str = m[0];
            auto start = str.find_last_of("\f\n\r\t\v ");
            start = (start == std::string::npos)?start:start+1;
            return str.substr(start);
        }
        return "";
    }
} // namespace helper

std::vector<VertextAssignment> ve::findAllOutVertexAssignments(const std::string& sourceCode)
{
    std::vector<VertextAssignment> results;
    // Search for all assignments into gl_Position
    auto vertexTransformationPattern  = std::regex("gl_Position[\f\n\r\t\v ]*=[\f\n\r\t\v ]*[^;]*;", std::regex::extended);
    std::smatch assignments;
    std::regex_search(sourceCode, assignments, vertexTransformationPattern);

    if(assignments.size() == 0)
        return {};
    for(const auto& statement: assignments)
    {
        VertextAssignment outputAssignment;
        outputAssignment.statementRawText = statement;
        outputAssignment.firstTokenFromLeft = helper::getFirstTokenAfterEq(statement);
        results.push_back(outputAssignment);
    }
    return results;
}
