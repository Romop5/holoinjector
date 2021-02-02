#include "pipeline/shader_inspector.hpp"
#include <regex>
#include <unordered_set>
#include <cassert>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <iostream>

#include "pipeline/shader_parser.hpp"

using namespace ve;
using namespace ve::pipeline;

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
        if(assignment.find(". xyww") != std::string::npos)
        {
            semicolon = output.find(". xyww");
        }

        output.insert(semicolon, std::string(")"));
        return output;
    }
} // namespace helper

bool ve::pipeline::ShaderInspector::isIdentifier(const std::string_view& token) const
{
    auto tk = std::string(token);
    std::smatch m;
    static auto identifier = std::regex("[a-zA-Z][a-zA-Z0-9_]*");
    return std::regex_match(tk, m, identifier);
}

bool ve::pipeline::ShaderInspector::isUniformVariableInInterfaceBlock(const std::string& identifier) const
{
    std::string regexLiteral = std::string("uniform[^;]*\\{[^\\}]*[\f\n\r\t\v ]") + identifier + std::string("[\f\n\r\t\v ]*;[^\\}]*\\}");
    auto isDefinedAsUniform = std::regex(regexLiteral,std::regex::extended);
    std::smatch m;
    std::regex_search(sourceCode, m, isDefinedAsUniform); 
    return m.size() > 0;
}

std::string ve::pipeline::ShaderInspector::getUniformBlockName(const std::string& uniformName) const
{
    if(uniformName.empty())
        return "";
    std::string rgxLiteral = std::string("uniform[^;]*[\f\n\r\t\v ]([a-zA-Z][a-zA-Z0-9_]*)[^;]*\\{[^\\}]*[\f\n\r\t\v ]") + uniformName + std::string("[\f\n\r\t\v ]*;[^\\}]*\\}");
    auto finalregex = std::regex(rgxLiteral,std::regex::extended);
    std::smatch matches;

    if(std::regex_search(sourceCode, matches, finalregex)) {
        if(matches.size() > 1)
            return matches[1];
    }
    return "";
}

bool ve::pipeline::ShaderInspector::isUniformVariable(const std::string& identifier) const
{
    if(identifier.empty())
        return false;

    if(isUniformVariableInInterfaceBlock(identifier))
            return true;
    std::string regexLiteral = std::string("uniform[^;]*[\f\n\r\t\v ]") + identifier + std::string("[\f\n\r\t\v ]*;");
    auto isDefinedAsUniform = std::regex(regexLiteral,std::regex::extended);
    std::smatch m;
    std::regex_search(sourceCode, m, isDefinedAsUniform); 
    return m.size() > 0;
}


std::string ve::pipeline::ShaderInspector::getVariableType(const std::string& variable) const
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

std::vector<ShaderInspector::VertextAssignment> ve::pipeline::ShaderInspector::findAllOutVertexAssignments() const
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
        outputAssignment.positionInCode = assignments.position();
        outputAssignment.statementRawText = foundText;
        outputAssignment.analysis = analyzeGLPositionAssignment(foundText);
        switch(outputAssignment.analysis.type)
        {
            case UNIFORM:
                outputAssignment.transformName = outputAssignment.analysis.foundIdentifier;
                break;
            case POSSIBLE_TEMPORARY_VARIABLE:
                // Special case: legacy OpenGL
                if(outputAssignment.analysis.foundIdentifier == "gl_ModelViewProjectionMatrix")
                {
                    outputAssignment.transformName = "";
                    outputAssignment.isFixedPipelineUsed = true;
                    break;
                }
                outputAssignment.transformName = recursivelySearchUniformFromTemporaryVariable(outputAssignment.analysis.foundIdentifier);

                if(!outputAssignment.transformName.empty())
                    outputAssignment.analysis.type = UNIFORM;
                break;
            case FUNCTION:
            {
                if(outputAssignment.analysis.foundIdentifier == "ftransform")
                {
                    outputAssignment.isFixedPipelineUsed = true;
                }
                auto fallback = getFallbackUniformViaHeuristic();
                if(!fallback.empty())
                {
                    outputAssignment.analysis.type = UNIFORM;
                    outputAssignment.transformName = fallback;
                }
            }
            break;
            default:
                outputAssignment.transformName = "";
                break;
        }
        results.push_back(outputAssignment);

        toBeSearched = assignments.suffix();
    }
    return results;
}


std::string ve::pipeline::ShaderInspector::injectShader(const std::vector<ShaderInspector::VertextAssignment>& assignments)
{
    std::string output = sourceCode;
    for(auto& statement: assignments)
    {
        const auto newStatement = replaceGLPositionAssignment(statement);
        auto startPosition = output.find(statement.statementRawText);
        if(startPosition == std::string::npos)
            continue;
        output.replace(startPosition,statement.statementRawText.length(),newStatement);
    }
    // Find first new line (typically, after #version and after #extension tag)
    auto lastMacroPosition = output.rfind("#");
    auto startOfFunction = output.find("\n", lastMacroPosition == std::string::npos?0:lastMacroPosition);
    assert(startOfFunction != std::string::npos);
    startOfFunction = output.rfind("\n", startOfFunction);

    // At this point, caller must have verified that this is a VS, containing void main() method
    assert(startOfFunction != std::string::npos);

    auto code = getCommonTransformationShader();
    output.insert(startOfFunction, code);
    return output;
}

std::string ve::pipeline::ShaderInspector::getTransformationUniformName(std::vector<VertextAssignment> assignments)
{
    for(const auto& statement: assignments)
    {
        if(isUniformVariable(statement.transformName))
            return statement.transformName;
    }
    return "";
}


/// Get count of declared uniforms in shader
size_t ve::pipeline::ShaderInspector::getCountOfUniforms() const
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

std::vector<std::pair<std::string, std::string>> ve::pipeline::ShaderInspector::getListOfUniforms() const
{
    std::vector<std::pair<std::string, std::string>> result;
    size_t position = sourceCode.find("uniform");
    while(position != std::string::npos)
    {
        auto positionSemicolon = sourceCode.find_first_of(";",position+1);
        auto positionBracket = sourceCode.find_first_of("{",position+1);

        if(positionBracket < positionSemicolon)
        {
            // parser shader block
            auto positionBracketEnd = sourceCode.find_first_of("}",position+1);
            auto uniformDefinition = sourceCode.substr(position, positionBracketEnd-position);
            auto tokens = ve::pipeline::tokenize(uniformDefinition);
            decltype(tokens)::iterator semicolon = tokens.begin();
            while((semicolon = std::find(semicolon, tokens.end(), ";")) != tokens.end())
            {
                result.emplace_back(std::make_pair(*(semicolon-2), *(semicolon-1)));
                semicolon += 1;
            }

        } else {
            // parse scalar uniform definition
            auto uniformDefinition = sourceCode.substr(position, positionSemicolon-position);
            auto definitionTokens = ve::pipeline::tokenize(uniformDefinition);
            const auto& type = definitionTokens[definitionTokens.size()-2];
            const auto& name = definitionTokens[definitionTokens.size()-1];

            result.emplace_back(std::make_pair(type, name));
        }

        position = sourceCode.find("uniform",position+1);
    } 
    return result;
}

std::vector<std::pair<std::string, std::string>> ve::pipeline::ShaderInspector::getListOfInputs() const
{
    std::vector<std::pair<std::string, std::string>> result;

    std::smatch m;
    static auto inDefinition = std::regex("[\f\n\r\t\v ]in[\f\n\r\t\v ][^;]+");

    auto searchIn = sourceCode;
    while(std::regex_search(searchIn, m, inDefinition))
    {
        for(auto& match: m)
        {
            std::string s = match.str();
            if(s.find_first_of("{}") != std::string::npos)
            {
                auto start = sourceCode.find("in", m.position());
                assert(start != std::string::npos);
                // skip 'in'
                start += 2;
                auto end = sourceCode.find("}", m.position());
                assert(end != std::string::npos);
                end += 1;
                auto idEnd = sourceCode.find(";", end);

                auto snippet = sourceCode.substr(start, idEnd-start);
                auto definitionTokens = ve::pipeline::tokenize(snippet);

                const auto type = sourceCode.substr(start, end-start);
                const auto& name = definitionTokens[definitionTokens.size()-1];
                assert(name.find_first_of(")(,;") == std::string::npos);
                result.emplace_back(std::make_pair(type, name));
                continue;
            }

            // is interface
            auto definitionTokens = ve::pipeline::tokenize(s);
            assert(definitionTokens.size() >= 2);
            const auto& type = definitionTokens[definitionTokens.size()-2];
            const auto& name = definitionTokens[definitionTokens.size()-1];

            assert(type.find_first_of(")(,;") == std::string::npos);
            assert(name.find_first_of(")(,;") == std::string::npos);
            result.emplace_back(std::make_pair(type, name));
        }
        searchIn = m.suffix();
    }
    return result;
}

std::vector<std::pair<std::string, std::string>> ve::pipeline::ShaderInspector::getListOfOutputs() const
{
    std::vector<std::pair<std::string, std::string>> result;

    std::smatch m;
    static auto inDefinition = std::regex("[\f\n\r\t\v ]out[\f\n\r\t\v ][^;]+");

    auto searchIn = sourceCode;
    while(std::regex_search(searchIn, m, inDefinition))
    {
        for(auto& match: m)
        {
            std::string s = match.str();
            auto definitionTokens = ve::pipeline::tokenize(s);
            assert(definitionTokens.size() >= 2);
            const auto& type = definitionTokens[definitionTokens.size()-2];
            const auto& name = definitionTokens[definitionTokens.size()-1];

            assert(type.find_first_of(")(,;") == std::string::npos);
            assert(name.find_first_of(")(,;") == std::string::npos);
            result.emplace_back(std::make_pair(type, name));
        }
        searchIn = m.suffix();
    }
    return result;
}

ShaderInspector::Analysis ve::pipeline::ShaderInspector::analyzeGLPositionAssignment(std::string& assignment) const
{
    auto tokens = ve::pipeline::tokenize(assignment);

    auto firstIdentifier = std::find_if(tokens.begin()+1, tokens.end(), [&](const auto token)->bool {
            return isIdentifier(token) && !ve::pipeline::isBuiltinGLSLType(token);});

    const auto uniforms = getListOfUniforms();
    const auto inputs = getListOfInputs();

    auto uniformPosition = std::find_if(uniforms.begin(),uniforms.end(), [&](auto variable)->bool { return variable.second == *firstIdentifier;});

    auto inputPosition = std::find_if(inputs.begin(),inputs.end(), [&](auto variable)->bool { return variable.second == *firstIdentifier;});

    bool isUniform = (uniformPosition != uniforms.end());
    bool isInput = (inputPosition != inputs.end());
    bool isFunction = (*(firstIdentifier+1)== "(");
    bool isGLPosition = (*(firstIdentifier)== "gl_Position");
    bool isConstantAssignment = firstIdentifier == tokens.end();
    bool couldBeVariable = (!isUniform && !isInput && !isFunction && !isGLPosition && !isConstantAssignment);

    /*std::cout << "IsUniform " << isUniform << std::endl;
    std::cout << "IsInput " << isInput<< std::endl;
    std::cout << "IsFunction " << isFunction << std::endl;
    std::cout << "IsGLPosition" << isGLPosition<< std::endl;
    std::cout << "IsConstantAssignment" << isConstantAssignment << std::endl;
    std::cout << "MightBeAVariable" << couldBeVariable << std::endl;
    */
    Analysis ana;
    ana.foundIdentifier = (firstIdentifier == tokens.end())?"":*firstIdentifier;
    if(isUniform)
        ana.type = AnalysisType::UNIFORM;
    else if(isInput)
        ana.type = AnalysisType::INPUT;
    else if(isFunction)
        ana.type = AnalysisType::FUNCTION;
    else if(isGLPosition)
        ana.type = AnalysisType::GLPOSITION;
    else if(isConstantAssignment)
        ana.type = AnalysisType::CONSTANT_ASSIGNMENT;
    else 
        ana.type = AnalysisType::POSSIBLE_TEMPORARY_VARIABLE;
    return ana;
}

std::string ve::pipeline::ShaderInspector::replaceGLPositionAssignment(VertextAssignment assignment) const
{
    if(assignment.isFixedPipelineUsed)
        return assignment.statementRawText;

    switch(assignment.analysis.type)
    {
        case UNIFORM:
            return helper::wrapAssignmentExpresion(assignment.statementRawText, "enhancer_VStransform");
        case POSSIBLE_TEMPORARY_VARIABLE:
        case INPUT:
            break;
            ///return helper::wrapAssignmentExpresion(assignment.statementRawText, "enhancer_transform_HUD");
        case GLPOSITION:
        case FUNCTION:
        default:
            // Identity, TODO: this would require more robust analysis
            return assignment.statementRawText;
    }
    return assignment.statementRawText;
}


std::string ShaderInspector::recursivelySearchUniformFromTemporaryVariable(std::string tmpName, size_t level) const
{
    if(level == 0)
        return "";
    auto firstTokenPattern = std::regex(tmpName+"[\f\n\r\t\v ]*=[^;]*");
    std::smatch m;
    while(std::regex_search(sourceCode, m, firstTokenPattern))
    {
        std::string foundStr = m.str();
        auto tokens = ve::pipeline::tokenize(foundStr);

        auto firstIdentifier = std::find_if(tokens.begin()+1, tokens.end(), [&](const auto token)->bool {
            return isIdentifier(token) && !ve::pipeline::isBuiltinGLSLType(token);});

        if(firstIdentifier != tokens.end())
        {
            const auto& name = std::string(*firstIdentifier);
            if(name == tmpName)
                continue;
            if(isUniformVariable(name))
                return name;
            return recursivelySearchUniformFromTemporaryVariable(name,level-1);
        }
    }
    return "";
}


std::string ShaderInspector::getFallbackUniformViaHeuristic() const
{
    const auto& list = getListOfUniforms();

    const std::vector<std::string> heuristicHints = {"proj", "MVP", "VP"};
    for(const auto& hint: heuristicHints)
    {
        auto result = std::find_if(list.begin(),list.end(), [&](const auto pair)->bool
        {
            // if uniform contains part of hint
            return (pair.first == "mat4" && pair.second.find(hint) != std::string::npos);
        });
        if(result != list.end())
            return (*result).second;
    }
    return "";
}

bool ShaderInspector::isClipSpaceShader() const
{
    static auto clipAssign = std::regex(".[\f\n\r\t\v ]*xyww");
    std::smatch m;
    return std::regex_search(sourceCode, m, clipAssign); 
}

void ShaderInspector::injectCommonCode(std::string& sourceOriginal)
{
    auto pos = sourceOriginal.find_first_of("\n", sourceOriginal.find_last_of("#"));
    assert(pos != std::string::npos);
    pos += 1;
    sourceOriginal.insert(pos, getCommonTransformationShader());
}

std::string ShaderInspector::getCommonTransformationShader()
{   
    static std::string code = R"(
    uniform int enhancer_cameraId = 0;
    uniform int enhancer_max_views = 9;
    uniform bool enhancer_isSingleViewActivated = false;
    uniform int enhancer_singleViewID = 0;

    uniform float enhancer_XShiftMultiplier = 5.0;
    uniform float enhancer_FrontalDistance = 5.0;

    uniform bool enhancer_isOrthogonal = false; 
    // when true, keeps original transformation flowing => used for shadow maps
    uniform bool enhancer_identity = true;

    // Contains (fx,fy, near,far), estimated from original projection
    uniform vec4 enhancer_deprojection;

    // 1/fx, etc...
    uniform vec4 enhancer_deprojection_inv;

    float enhancer_normalizedCamera(int cameraId)
    {
        return 1.0-2.0*float(cameraId+1)/float(enhancer_max_views+1);
    }
    float enhancer_getProjectionShift(int cameraId)
    {
        float normalizedDistance = 1.0-2.0*float(cameraId+1)/float(enhancer_max_views+1);
        return normalizedDistance*enhancer_XShiftMultiplier/enhancer_FrontalDistance;
    }

    float enhancer_getShearCoeff(int cameraId, float r)
    {
        float normalizedDistance = 1.0-2.0*float(cameraId+1)/float(enhancer_max_views+1);
        return 1.0*normalizedDistance*enhancer_XShiftMultiplier/r;
    }


    float enhancer_getCenterShift(int cameraId)
    {
        float normalizedDistance = 1.0- 2.0*float(cameraId+1)/float(enhancer_max_views+1);
        return normalizedDistance*enhancer_XShiftMultiplier;
    }

    // Reversts original projection and apple per-view transformation & projection
    vec4 enhancer_extractViewSpace(vec4 clipSpace)
    {
	if(enhancer_isOrthogonal || enhancer_identity)
            return clipSpace;

        // Revert clip-space to view-space
        vec4 viewSpace = vec4(clipSpace.x*enhancer_deprojection_inv[0], clipSpace.y*enhancer_deprojection_inv[1], -clipSpace.w,1.0);
	return viewSpace;
    }
    vec4 enhancer_transform(bool isClipSpace, int camera, vec4 clipSpace)
    {
        if(enhancer_isOrthogonal || enhancer_identity)
            return clipSpace;

        float nearOriginal = enhancer_deprojection[2];
        //float near = enhancer_FrontalDistance;
        float near = nearOriginal;
        float far = enhancer_deprojection[3];
        
        float A = -2.0/(far-near);
        float B = -(far+near)/(far-near);

        const float aspect = 1.3;
        float angle = radians(80.0);

        float r = tan(angle*0.5)*near;

        float fovX = enhancer_deprojection[0];
        float fovY = enhancer_deprojection[1];
        //float fovX = near/r;
        //float fovY = fovX*aspect;
        
        mat4 projection = mat4(
            vec4(fovX,0.0f,0.0,0.0),
            vec4(0.0f,fovY,0.0f,0.0),
            vec4(enhancer_getProjectionShift(camera),0.0f,A,-1.0),
            vec4(0,0.0f,B,0.0));

        // Do per-view transformation
        vec4 viewSpace = enhancer_extractViewSpace(clipSpace);
        if(!isClipSpace)
            viewSpace.x += enhancer_getCenterShift(camera);
        vec4 viewSpaceTransformed = projection*viewSpace;
        if(isClipSpace)
            return viewSpaceTransformed.xyww;
        return viewSpaceTransformed;
    }

    vec4 enhancer_VStransform(vec4 clipSpace)
    {
        return clipSpace;
    }
    )";

    // Hack: remove clip space
    code= std::regex_replace(code, std::regex(". xyww"),"");
    return code;
}
