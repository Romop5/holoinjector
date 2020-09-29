#include "shader_inspector.hpp"
#include <regex>
#include <unordered_set>
#include <cassert>

using namespace ve;

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

    /// Given "X = Y;", do "X = expression (Y);"
    std::string wrapAssignmentExpresion(const std::string assignment, const std::string expression)
    {
        std::string output = assignment;
        auto equalSign = output.find("=")+1;
        output.insert(equalSign, expression+std::string("("));

        auto semicolon = output.rfind(";");
        output.insert(semicolon, std::string(")"));
        return output;
    }

    size_t findFirstNonwhitespaceCharacter(const std::string_view& str)
    {
        auto result = std::find_if(str.begin(),str.end(), std::isspace());
        if(result == str.end())
            return std::string::npos;
        return (result-str.begin());
    }
    size_t findEndOfToken(const std::string_view& str)
    {
        enum TokenType
        {
            IDENDIFIER, // [a-zA-Z0-9]
            OPERATOR, // * - + /
            SEMICOLON, // ;
            BRACE, // { } [ ] ( )
            DOT, // .
            LITERAL // [0-9]\+(.[0-9]*(f)?)?
        };
        size_t endPosition = str.size();
        for(auto it = str.begin();  it < str.end(); it++)
        {
            if(std::isspace(*it))
                break;
        }

    }
    std::vector<std::string_view> whitespaceSeparatedTokens(const std::string& code)
    {
        auto remainingString = code;
        auto startingPosition = 0;
        while((startingPosition = helper::findFirstNonwhitespaceCharacter(remainingString))
                != std::string::npos)
        {
            enum TokenType
            {

            };

            auto tokenStartPosition = startingPosition;
            for(auto it = code.begin()+startingPosition;  it < code.end(); it++)
            {
                if(std::isspace(*it))
                    break;
            }
        }
    }
} // namespace helper



bool ve::ShaderInspector::isBuiltinGLSLType(const std::string& token)
{
    static const auto builtinTypes = std::unordered_set<std::string>{ "vec3", "vec4", "mat3", "mat4", "float", "double"};
    return (builtinTypes.count(token) > 0);
}

bool ve::ShaderInspector::isUniformVariableInInterfaceBlock(const std::string& identifier) const
{
    std::string regexLiteral = std::string("uniform[^;]*\\{[^\\}]*[\f\n\r\t\v ]") + identifier + std::string("[\f\n\r\t\v ]*;[^\\}]*\\}");
    auto isDefinedAsUniform = std::regex(regexLiteral,std::regex::extended);
    std::smatch m;
    std::regex_search(sourceCode, m, isDefinedAsUniform); 
    return m.size() > 0;
}


bool ve::ShaderInspector::isUniformVariable(const std::string& identifier) const
{
    if(isUniformVariableInInterfaceBlock(identifier))
            return true;
    std::string regexLiteral = std::string("uniform[^;]*[\f\n\r\t\v ]") + identifier + std::string("[\f\n\r\t\v ]*;");
    auto isDefinedAsUniform = std::regex(regexLiteral,std::regex::extended);
    std::smatch m;
    std::regex_search(sourceCode, m, isDefinedAsUniform); 
    return m.size() > 0;
}


std::string ve::ShaderInspector::getVariableType(const std::string& variable) const
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

std::vector<ShaderInspector::VertextAssignment> ve::ShaderInspector::findAllOutVertexAssignments() const
{
    std::vector<VertextAssignment> results;
    // Search for all assignments into gl_Position
    auto vertexTransformationPattern  = std::regex("gl_Position[\f\n\r\t\v ]*=[\f\n\r\t\v ]*[^;]*;", std::regex::extended);
    std::smatch assignments;
    std::string toBeSearched = sourceCode;
    while(std::regex_search(toBeSearched, assignments, vertexTransformationPattern))
    {
        std::string foundText = assignments.str();
        VertextAssignment outputAssignment;
        outputAssignment.statementRawText = foundText;
        outputAssignment.firstTokenFromLeft = helper::getFirstTokenAfterEq(foundText);
        results.push_back(outputAssignment);

        toBeSearched = assignments.suffix();
    }
    return results;
}


std::string ve::ShaderInspector::injectShader(const std::vector<ShaderInspector::VertextAssignment>& assignments)
{
    std::string output = sourceCode;
    for(auto& statement: assignments)
    {
        const auto isUniform = isUniformVariable(statement.firstTokenFromLeft);
        const auto isBuiltin= isBuiltinGLSLType(statement.firstTokenFromLeft);
        if(!isBuiltin && !isUniform)
            continue;
        const auto enhancerFunctionName = isBuiltinGLSLType(statement.firstTokenFromLeft)?"enhancer_transform_HUD": "enhancer_transform";
        const auto newStatement = helper::wrapAssignmentExpresion(statement.statementRawText, enhancerFunctionName);
        auto startPosition = output.find(statement.statementRawText);
        if(startPosition == std::string::npos)
            continue;
        output.replace(startPosition,statement.statementRawText.length(),newStatement);
    }
    auto startOfFunction = output.find("main");
    assert(startOfFunction != std::string::npos);
    startOfFunction = output.rfind("\n", startOfFunction);

    // At this point, caller must have verified that this is a VS, containing void main() method
    assert(startOfFunction != std::string::npos);
    static std::string code = R"(
        uniform bool enhancer_isOrthogonal; 
        // when true, keeps original transformation flowing => used for shadow maps
        uniform bool enhancer_identity; 
        // Contains PROJ*VIEW per-view camera
        uniform mat4 enhancer_view_transform; 

        // Contains (fx,fy, near,far), estimated from original projection
        uniform vec4 enhancer_estimatedParameters;

        // Reversts original projection and apple per-view transformation & projection
        vec4 enhancer_transform(vec4 clipSpace)
        {
            if(enhancer_identity)
                return clipSpace;
            float fx = enhancer_estimatedParameters[0];
            float fy = enhancer_estimatedParameters[1];
            float near = enhancer_estimatedParameters[2];
            float far = enhancer_estimatedParameters[3];
            // Revert clip-space to view-space
            vec4 viewSpace = vec4(clipSpace.x/fx, clipSpace.y/fy, -clipSpace.w,1.0);

            if(enhancer_isOrthogonal)
            {
                viewSpace[2] = 0;
            }

            float A = -2.0/(far-near);
            float B = -(far+near)/(far-near);
            if(enhancer_isOrthogonal)
            {
                A = 1.0;
                B = 0.0;
            }
            mat4 projection = mat4(vec4(fx,0.0f,0.0f,0.0), vec4(0.0f,fy,0.0f,0.0), vec4(0.0f,0.0f,A,-1.0), vec4(0.0f,0.0f,B,0.0));
            if(enhancer_isOrthogonal)
            {
                projection[3][3] = 1.0;
                projection[2][3] = 0.0;
            }

            // Do per-view transformation
            vec4 viewSpaceTransformed = projection*enhancer_view_transform * viewSpace;
            return viewSpaceTransformed;
        }

        // Transform without deprojecting
        vec4 enhancer_transform_HUD(vec4 clipSpace)
        {
            if(enhancer_identity)
                return clipSpace;
            // Revert clip-space to view-space
            return enhancer_view_transform*clipSpace;
        }
    )";

    output.insert(startOfFunction, code);
    return output;
}

std::string ve::ShaderInspector::getTransformationUniformName(std::vector<VertextAssignment> assignments)
{
    for(const auto& statement: assignments)
    {
        if(isUniformVariable(statement.firstTokenFromLeft))
            return statement.firstTokenFromLeft;
    }
    return "";
}


/// Get count of declared uniforms in shader
size_t ve::ShaderInspector::getCountOfUniforms() const
{
    size_t count = 0;
    size_t position = sourceCode.find("uniform");
    while(position != std::string::npos)
    {
        count++;
        position = sourceCode.find("uniform",position+1);
    } 
    return count;
}

std::vector<std::pair<std::string, std::string>> Repeater::getListOfUniforms() const
{
    size_t count = 0;
    size_t position = sourceCode.find("uniform");
    while(position != std::string::npos)
    {
        count++;
        position = sourceCode.find("uniform",position+1);
        positionSemicolon = sourceCode.find_first_of(";",position+1);
        auto uniformDefinition = sourceCode.substr(position, positionSemicolon-position);
    } 
    return count;
}

