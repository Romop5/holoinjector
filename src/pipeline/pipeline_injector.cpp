#include "pipeline/pipeline_injector.hpp"
#include "pipeline/shader_inspector.hpp"

#include <regex>
#include <cassert>

using namespace ve;

PipelineInjector::PipelineProcessResult PipelineInjector::process(PipelineType input, const PipelineParams& params)
{
    PipelineType output = input;
    auto metadata = std::make_unique<ProgramMetadata>();
    bool hasFilledMetadata = false;

    assert(output.count(GL_FRAGMENT_SHADER) > 0);
    assert(output.count(GL_VERTEX_SHADER) > 0);

    /*
     * Inspect original shader program and search for transformation
     */
    if(output.count(GL_GEOMETRY_SHADER) > 0)
    {   // Try Geometry shader at first
        auto GS = output.at(GL_GEOMETRY_SHADER);
        if(injectShader(GS, *metadata))
        {
            hasFilledMetadata = true;
            output[GL_GEOMETRY_SHADER] = GS;
        }
    }

    auto VS = output.at(GL_VERTEX_SHADER);
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
    
    // Inject new GS if none is provided
    if(input.count(GL_GEOMETRY_SHADER) == 0)
        output = insertGeometryShader(output,updatedParams);
    else 
        output = injectGeometryShader(output,updatedParams);
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

    geometryShaderStream << ShaderInspector::getCommonTransformationShader() << "\n";
    geometryShaderStream << "layout (triangle_strip, max_vertices = " << 3*params.countOfInvocations <<  ") out;\n";
    geometryShaderStream << "layout (invocations= " << params.countOfInvocations <<  ") in;\n";
    geometryShaderStream << "const bool enhancer_geometry_isClipSpace = " 
        << (params.shouldRenderToClipspace?"true":"false") <<";\n";
    geometryShaderStream << "const int enhancer_max_invocations = " << params.countOfInvocations << ";\n";
    geometryShaderStream << "const int enhancer_duplications = "<< params.countOfPrimitivesDuplicates << ";\n";
    geometryShaderStream << R"(
        void identity_main(int layer)
        {
            int enhancer_camera_id = 0;
            
            // identity shader
            int i = 0;
            for(i = 0; i < 3; i++)
            {
                gl_Position = gl_in[i].gl_Position;
                gl_Layer = layer;
		gl_Position = enhancer_transform(enhancer_geometry_isClipSpace,layer,gl_Position);
                EmitVertex();
            }
            EndPrimitive();
        }

        void main()
        {
            // Allows to override max_invocations using uniform
            if(gl_InvocationID >= enhancer_max_invocations)
                return;
            int layer = gl_InvocationID*enhancer_duplications;
            for(int i = 0; i < enhancer_duplications; i++)
            {
                if(layer+i >= enhancer_max_views)
                    return;
                identity_main(layer+i);
            }
        }
    )";
    auto geometryShader = std::move(geometryShaderStream.str());

    auto FS = pipeline.at(GL_FRAGMENT_SHADER);
    ShaderInspector inspector(FS);
    auto inputs = inspector.getListOfInputs();
    std::string ioDefinitionString; 
    std::string ioRedirections;
    /*
     * For each "in type name;" in Fragment Shader
     * Create "in type name[]; out type enhancer_frag_name;"
     * +
     * create "enhancer_frag_name = name[i];
     */
    for(auto& [type,name]: inputs)
    {
        ioDefinitionString += "in " + type + " " + name + "[];\n";
        ioDefinitionString += "out " + type + " " + "enhancer_frag_"+name + ";\n";

        ioRedirections += "enhancer_frag_"+name+" = " + name + "[i];\n";
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
    auto fragmentShader = result[GL_FRAGMENT_SHADER]; 
    for(auto& [_,name]: inputs)
    {
        fragmentShader = std::regex_replace(fragmentShader, std::regex(name),"enhancer_frag_"+name);  
    }
     
    /*
     * Store Geometry shader
     */
    result[GL_GEOMETRY_SHADER] = geometryShader;
    result[GL_FRAGMENT_SHADER] = fragmentShader;
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

    // 1. rename void main() to void old_main()
    assert(geometryShader.find("void main") != std::string::npos);
    geometryShader = std::regex_replace(geometryShader, std::regex("void main()"),"void old_main(int enhancer_layer)");  
    
    // 2. Add gl_Layer = enhancer_layer; before each EmitVertex
    geometryShader = std::regex_replace(geometryShader, std::regex("EmitVertex"),"gl_Layer = enhancer_layer; \nEmitVertex");  
    // 3. insert double the 'max_vertices' count
    auto maxVerticesPosition = geometryShader.find("max_vertices");
    assert(maxVerticesPosition != std::string::npos);

    auto eqPosition = geometryShader.find("=", maxVerticesPosition);

    geometryShader.insert(eqPosition+1, std::string(" ")+std::to_string(params.countOfPrimitivesDuplicates)+" *"); 
    // 4. insert invocations
    // finds the old main() function
    auto beforeMainFunctionPosition = geometryShader.find("void");
    geometryShader.insert(beforeMainFunctionPosition, std::string("layout(invocations = ")+std::to_string(params.countOfInvocations)+") in\n");


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
            // TODO: layer not bound
            int layer = gl_InvocationID*maxDuplications+duplicationId;
            old_main(layer);
        }
    }
    )";

    // Place new main() into the end of shader
    geometryShader.insert(geometryShader.size()-1, newMainFunction);

    // Replace version with GLSL 4.6
    geometryShader = std::regex_replace(geometryShader, std::regex("#version[^\n]\n"),"#version 440 core\n");  

    // Return new pipeline
    auto output = pipeline;
    output[GL_GEOMETRY_SHADER] = geometryShader;
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
    outMetadata.m_InterfaceBlockName = inspector.getUniformBlockName(outMetadata.m_TransformationMatrixName);
    outMetadata.m_HasAnyUniform = (inspector.getCountOfUniforms() > 0);

    sourceCode = inspector.injectShader(statements);
    return true;
}
