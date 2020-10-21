#include "pipeline_injector.hpp"
#include "shader_inspector.hpp"

#include <regex>

using namespace ve;

PipelineInjector::PipelineType PipelineInjector::process(PipelineType input)
{
    PipelineType output;
    // Inject new GS if none is provided
    if(input.count(GL_GEOMETRY_SHADER) == 0)
    {
        output = injectGeometryShader(input,GSInjectionParams{});
    }
    return output;
}

PipelineInjector::PipelineType PipelineInjector::injectGeometryShader(const PipelineType& pipeline, const GSInjectionParams params)
{
    auto result = pipeline;
    std::stringstream geometryShaderStream;
    geometryShaderStream <<  R"(
        #version 430 compatible 
        layout (triangles) in;
        layout (triangles, max_vertices = 3) out;

        )";
    geometryShaderStream << "const enhancer_geometry_isClipSpace = " << params.shouldRenderToClipspace <<";\n";
    geometryShaderStream << R"(
        void main()
        {
            int enhancer_camera_id = 0;
            
            // identity shader
            int i = 0;
            for(i = 0; i < 3; i++)
            {
                gl_Position = gl_in[i].gl_Position;
                if(enhancer_geometry_isClipSpace)
                {
                    gl_position = vec4(gl_Position.xy, 1.0, 1.0);
                }
                EmitVertex();
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
    auto pos = geometryShader.find("void main()");
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
