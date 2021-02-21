#include "pipeline/pipeline_injector.hpp"
#include "pipeline/shader_inspector.hpp"
#include "pipeline/shader_parser.hpp"

#include <regex>
#include <cassert>

using namespace ve;
using namespace ve::pipeline;

constexpr bool shouldUseOnlyVS = false;

namespace helper
{
    /// Iterate&replace over generic text, matched by regex
    std::string regex_replace_functor(const std::string str, const std::regex reg, std::function<std::string(std::string)> functor)
    {
        std::string result = str;
        size_t startPosition = 0;
        size_t currentLength = result.size();
        auto end= std::sregex_iterator();
        do {
            auto it = std::sregex_iterator(result.begin()+startPosition, result.end(), reg);
            if(it == end)
                break;
            auto& match = *it;
            auto newString = functor(match.str(0));
            // replace original with new
            result.replace(startPosition+match.position(0), match.str(0).size(), newString);
            // advance pass the replaced string
            startPosition += match.position(0)+newString.size();
            size_t currentLength = result.size();
        } while (startPosition < currentLength);
        return result;
    }
    
    /// Iterate&replace over all identifiers in text
    std::string regex_replace_identifiers(const std::string str, const std::string identifierName, std::function<std::string()> functor)
    {
        auto replaceRegexSearch = std::regex(std::string("([a-zA-Z_-][a-zA-Z0-9_-]*)?")+identifierName+std::string("[a-zA-Z0-9_-]*"));
        return helper::regex_replace_functor(str, replaceRegexSearch,[&](auto str)->std::string
        {
            if(str == identifierName)
                return functor();
            return str;
        });
    }

}

PipelineInjector::PipelineProcessResult PipelineInjector::process(PipelineType input, const PipelineParams& params)
{
    PipelineType output = input;
    auto metadata = std::make_unique<ProgramMetadata>();
    bool hasFilledMetadata = false;

    /*
     * Detect if pipeline is drawable
     */
    if(output.count(GL_VERTEX_SHADER) == 0 || output.count(GL_FRAGMENT_SHADER) == 0)
    {
        // Identity: pipeline is not drawable, so it's return as it is.
        return {input, nullptr};
    }

    assert(output.count(GL_FRAGMENT_SHADER) > 0);
    assert(output.count(GL_VERTEX_SHADER) > 0);

    /*
     * Inspect original shader program and search for transformation
     */
    if(output.count(GL_GEOMETRY_SHADER) > 0)
    {   // Try Geometry shader at first
        auto GS = output.at(GL_GEOMETRY_SHADER);
        auto isLayeredProgram = GS.find("gl_Layer");
        if(isLayeredProgram)
        {   // if original geometry shader already uses gl_Position, it's probably cube map rendering
            // so we can't change the structure of program
            return {input, nullptr};
        }
        // does Geometry Shader calculate MVP transformation?
        if(injectShader(GS, *metadata))
        {
            hasFilledMetadata = true;
            output[GL_GEOMETRY_SHADER] = GS;
        } else {
            ShaderInspector::injectCommonCode(output[GL_GEOMETRY_SHADER]);
        }
    }

    auto VS = output.at(GL_VERTEX_SHADER);
    // if geometry shader does not exist or it does not calculat MVP and VS does
    if(!hasFilledMetadata && injectShader(VS, *metadata))
    {   // try VS if GS does not exists or does not contain transformation
        hasFilledMetadata = true;
        output[GL_VERTEX_SHADER] = VS;
    }

    auto updatedParams = params;
    if(metadata)
    {
        updatedParams.shouldRenderToClipspace = metadata->m_IsClipSpaceTransform;
    }
    /*
     * In any case, inject geometry shader
     */
    
    metadata->m_IsGeometryShaderUsed = true;

    // Inject new GS if none is provided
    if(input.count(GL_GEOMETRY_SHADER) == 0)
    {
        if(shouldUseOnlyVS)
        {
            metadata->m_IsGeometryShaderUsed = false;
            output = injectVertexShader(output,updatedParams);
        }
        else
            output = insertGeometryShader(output,updatedParams);
    } 
    else {
        output = injectGeometryShader(output,updatedParams);
    }
    // Replace version with GLSL 4.6
    for(auto& [type, shaderSourceCode]: output)
    {
        if(type != GL_GEOMETRY_SHADER)
            continue;
        shaderSourceCode = std::regex_replace(shaderSourceCode, std::regex("version[\f\n\r\t\v ]*[0-9]*"), "version 450");
    }

    return {output, (hasFilledMetadata)?std::move(metadata):nullptr};
}

PipelineInjector::PipelineType PipelineInjector::insertGeometryShader(const PipelineType& pipeline, const PipelineParams params)
{
    // Verify that pipeline has FS and does not have GS
    assert(pipeline.count(GL_FRAGMENT_SHADER) == 1);
    assert(pipeline.count(GL_GEOMETRY_SHADER) == 0);

    // Construct new Geometry Shader
    auto result = pipeline;
    std::stringstream geometryShaderStream;
    geometryShaderStream <<  R"(
        #version 440 core
        layout (triangles) in;
        )";

    geometryShaderStream << "//------------ Enhancer Insert Header start\n";
    geometryShaderStream << ShaderInspector::getCommonTransformationShader() << "\n";
    geometryShaderStream << "layout (triangle_strip, max_vertices = " << 3*params.countOfPrimitivesDuplicates<<  ") out;\n";
    geometryShaderStream << "layout (invocations= " << params.countOfInvocations <<  ") in;\n";
    geometryShaderStream << "const bool enhancer_geometry_isClipSpace = "
        << (params.shouldRenderToClipspace?"true":"false") <<";\n";
    geometryShaderStream << "const int enhancer_max_invocations = " << params.countOfInvocations << ";\n";
    geometryShaderStream << "const int enhancer_duplications = "<< params.countOfPrimitivesDuplicates << ";\n";
    geometryShaderStream << R"(
        void identity_main(int layer)
        {
            // identity shader
            for(int i = 0; i < 3; i++)
            {
                gl_Position = gl_in[i].gl_Position;
                gl_Layer = (enhancer_isSingleViewActivated?enhancer_singleViewID:layer);
		gl_Position = enhancer_transform(enhancer_geometry_isClipSpace,layer,gl_Position);
                EmitVertex();
            }
            EndPrimitive();
        }

        void main()
        {
            // Allows to override max_invocations using uniform
            //if(gl_InvocationID >= enhancer_max_invocations)
            //    return;
            int layer = gl_InvocationID*enhancer_duplications;
            for(int i = 0; i < enhancer_duplications; i++)
            {
                if((layer+i) >= enhancer_max_views)
                    return;
                identity_main(layer+i);
            }
        }
    )";
    geometryShaderStream << "//------------ Enhancer Insert Header end\n";
    auto geometryShader = std::move(geometryShaderStream.str());

    /*
     * Remap 'varying' to out (VS) and in (FS) before replacing in/out
     */
    auto vertexShader = result[GL_VERTEX_SHADER];
    vertexShader= helper::regex_replace_identifiers(vertexShader,"varying", []{ return "out"; });

    auto fragmentShader = result[GL_FRAGMENT_SHADER];
    fragmentShader = helper::regex_replace_identifiers(fragmentShader,"varying", [] { return "in"; });

    ShaderInspector inspector(fragmentShader);

    auto inputs = inspector.getListOfInputs();
    std::string ioDefinitionString;
    std::string ioRedirections;
    /*
     * For each "in type name;" in Fragment Shader
     * Create "in type name[]; out type enhancer_frag_name;"
     * +
     * create "enhancer_frag_name = name[i];
     */
    for(const auto& [type,name]: inputs)
    {
        ioDefinitionString += "in " + type + " " + name + "[];\n";
        ioDefinitionString += "out " + type + " " + "enhancer_frag_"+name + ";\n";


        bool isInterfaceBlock = type.find_first_of("{}") != std::string::npos;
        if(!isInterfaceBlock)
        {
            ioRedirections += "enhancer_frag_"+name+" = " + name + "[i];\n";
        } else
        {
            /*
             * Interface blocks must be copied per-partes
             * => for each identifier, generate "out.id = in.id;" statement
             */
            auto interfaceBlockDefinitionTokens = ve::pipeline::tokenize(type);
            for(size_t i = 1; i < interfaceBlockDefinitionTokens.size(); i++)
            {
                if(interfaceBlockDefinitionTokens[i] != ";")
                    continue;
                std::stringstream ibRedirectionString;
                ibRedirectionString << "enhancer_frag_" << name << "." << interfaceBlockDefinitionTokens[i-1] <<
                    " = " << name <<  "[i]." << interfaceBlockDefinitionTokens[i-1] << ";\n";
                ioRedirections += ibRedirectionString.str();
            }
        }
    }

    /*
     * Insert in/out redirection in Geometry Shader
     */
    auto pos = geometryShader.find("void");
    geometryShader = geometryShader.insert(pos, ioDefinitionString);

    pos = geometryShader.find("EmitVertex");
    geometryShader = geometryShader.insert(pos, ioRedirections);

    /*
     * Replace all in attributes with prefixed version
     */
    for(const auto& [type,name]: inputs)
    {
        std::string inputName = name;
        bool isInterfaceBlock = type.find_first_of("{}") != std::string::npos;
        auto nameSuffix = (isInterfaceBlock?"_fs":"");
        fragmentShader = helper::regex_replace_identifiers(fragmentShader,inputName, [inputName, nameSuffix] { return "enhancer_frag_"+inputName+nameSuffix; });
    }
     
    /*
     * Store Geometry shader
     */
    result[GL_GEOMETRY_SHADER] = geometryShader;
    result[GL_FRAGMENT_SHADER] = fragmentShader;
    result[GL_VERTEX_SHADER] = vertexShader;
    return result;
}


PipelineInjector::PipelineType PipelineInjector::injectGeometryShader(const PipelineType& pipeline, const PipelineParams params)
{
    /*
     * TODO: following code does not expect output streams (as used by TransformFeedback mechanism)
     * => this could cause troubles in future
     */
    // Verify that pipeline has GS
    assert(pipeline.count(GL_GEOMETRY_SHADER) == 1);
    auto geometryShader = pipeline.at(GL_GEOMETRY_SHADER);

    // TODO: code is not ready for output streams
    assert(geometryShader.find("EmitStream") == std::string::npos);

    // 0. insert common shader functions / uniforms
    auto lastMacroDeclarationPosition = geometryShader.find_first_of("\n", geometryShader.find_last_of("#"));
    assert(lastMacroDeclarationPosition != std::string::npos);
    lastMacroDeclarationPosition += 1;

    std::stringstream headerInclude;
    headerInclude << "//------------ Enhancer Inject Header start\n";
    headerInclude << "int enhancer_layer = 0; \n"; 
    // Already injected by ShaderInspector::inject()
    //headerInclude << ShaderInspector::getCommonTransformationShader() << "\n";
    headerInclude << "//------------ Enhancer Inject Header end\n";
    geometryShader.insert(lastMacroDeclarationPosition, headerInclude.str());

    // 1. rename void main() to void old_main()
    assert(geometryShader.find("void main") != std::string::npos);
    geometryShader = std::regex_replace(geometryShader, std::regex("void[\f\n\r\t\v ]+main[\f\n\r\t\v ]*\\([\f\n\r\t\v ]*\\)"),"void old_main(int enhancer_layer)");


    
    // 2. Add gl_Layer = enhancer_layer; before each EmitVertex
    geometryShader = std::regex_replace(geometryShader, std::regex("EmitVertex"),"gl_Layer = (enhancer_isSingleViewActivated?enhancer_singleViewID:enhancer_layer); \nEmitVertex");
    geometryShader = std::regex_replace(geometryShader, std::regex("EmitVertex"),"gl_Position = enhancer_transform(enhancer_geometry_isClipSpace, enhancer_layer, gl_Position); \nEmitVertex");
    // 3. insert double the 'max_vertices' count
    // 3. insert double the 'max_vertices' count
    auto maxVerticesPosition = geometryShader.find("max_vertices");
    assert(maxVerticesPosition != std::string::npos);

    auto eqPosition = geometryShader.find("=", maxVerticesPosition);

    geometryShader.insert(eqPosition+1, std::string(" ")+std::to_string(params.countOfPrimitivesDuplicates)+" *"); 
    // 4. insert invocations
    // finds the old main() function
    std::stringstream beforeMainCodeChunk;
    beforeMainCodeChunk << "//------------ Enhancer Inject Header start\n";
    beforeMainCodeChunk << "layout(invocations = " << params.countOfInvocations << ") in;\n";
    beforeMainCodeChunk << "const bool enhancer_geometry_isClipSpace = " << (params.shouldRenderToClipspace?"true":"false") <<";\n";
    beforeMainCodeChunk << "//------------ Enhancer Inject Header end\n";

    auto beforeMainFunctionPosition = geometryShader.find("void");
    geometryShader.insert(beforeMainFunctionPosition, beforeMainCodeChunk.str());

    // 5. Insert new main() function, calling original main() for invocation*duplicate-times, setting correct
    // layer for each of call
    
    std::string newMainFunction =  
    std::string("const int maxDuplications = ") + std::to_string(params.countOfPrimitivesDuplicates) + ";\n" +
    R"(
    void main()
    {
        int duplicationId = 0;
        for(duplicationId = 0; duplicationId < maxDuplications; duplicationId++)
        {
            enhancer_layer = gl_InvocationID*maxDuplications+duplicationId;
            if(enhancer_layer>= enhancer_max_views)
                return;
            old_main(enhancer_layer);
        }
    }
    )";

    // Place new main() into the end of shader
    geometryShader.insert(geometryShader.size(), newMainFunction);


    // Return new pipeline
    auto output = pipeline;
    output[GL_GEOMETRY_SHADER] = geometryShader;
    return output;
}


PipelineInjector::PipelineType PipelineInjector::injectVertexShader(const PipelineType& pipeline, const PipelineParams params)
{
    // Verify that pipeline does not have 
    assert(pipeline.count(GL_GEOMETRY_SHADER) == 0);
    auto vertexShader = pipeline.at(GL_VERTEX_SHADER);

    // 0. insert common shader functions / uniforms
    auto lastMacroDeclarationPosition = vertexShader.find_first_of("\n", vertexShader.find_last_of("#"));
    auto hasNewlineAfterLastMacro = (lastMacroDeclarationPosition != std::string::npos);
    if(!hasNewlineAfterLastMacro)
    {
        // Insert in the beginning instead
        lastMacroDeclarationPosition = 0;
    }

    lastMacroDeclarationPosition += 1;

    std::stringstream headerInclude;
    headerInclude << "//------------ Enhancer Inject Header start\n";
    headerInclude << "int enhancer_layer = 0; \n"; 
    // Already injected by ShaderInspector::inject()
    //headerInclude << ShaderInspector::getCommonTransformationShader() << "\n";
    headerInclude << "//------------ Enhancer Inject Header end\n";
    vertexShader.insert(lastMacroDeclarationPosition, headerInclude.str());

    // 1. rename void main() to void old_main()
    assert(vertexShader.find("void main") != std::string::npos);
    vertexShader = std::regex_replace(vertexShader, std::regex("void[\f\n\r\t\v ]+main[\f\n\r\t\v ]*\\([^)]*\\)"),"void old_main()");

    // Assert that main was really replaced
    assert(vertexShader.find("void main") == std::string::npos);
    // 2. insert invocations
    // finds the old main() function
    std::stringstream beforeMainCodeChunk;
    beforeMainCodeChunk << "//------------ Enhancer Inject Header start\n";
    beforeMainCodeChunk << "const bool enhancer_geometry_isClipSpace = " << (params.shouldRenderToClipspace?"true":"false") <<";\n";
    beforeMainCodeChunk << "//------------ Enhancer Inject Header end\n";

    auto beforeMainFunctionPosition = vertexShader.find("void");
    vertexShader.insert(beforeMainFunctionPosition, beforeMainCodeChunk.str());

    // 3. Insert new main() function, computing shifted gl_Position
    std::string newMainFunction =
    R"(
    void main()
    {
        old_main();
        gl_Position = enhancer_transform(enhancer_geometry_isClipSpace,enhancer_singleViewID, gl_Position);
    }
    )";

    // Place new main() into the end of shader
    vertexShader.insert(vertexShader.size(), newMainFunction);

    // Return new pipeline
    auto output = pipeline;
    output[GL_VERTEX_SHADER] = vertexShader;
    return output;
}



bool PipelineInjector::injectShader(std::string& sourceCode, ProgramMetadata& outMetadata)
{
    ShaderInspector inspector(sourceCode);
    auto statements = inspector.findAllOutVertexAssignments();
    auto transformationName = inspector.getTransformationUniformName(statements);
    if(transformationName.empty())
        return false;
    
    outMetadata.m_TransformationMatrixName = transformationName;
    outMetadata.m_IsClipSpaceTransform = inspector.isClipSpaceShader();
    outMetadata.m_HasAnyFtransform = inspector.hasFtransform();
    outMetadata.m_InterfaceBlockName = inspector.getUniformBlockName(outMetadata.m_TransformationMatrixName);
    outMetadata.m_HasAnyUniform = (inspector.getCountOfUniforms() > 0);

    sourceCode = inspector.injectShader(statements);
    return true;
}
