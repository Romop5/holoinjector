#include "gtest/gtest.h"
#include "pipeline/pipeline_injector.hpp"
#include <GL/gl.h>
using namespace ve;
using namespace ve::pipeline;
namespace helper
{
    static void printPipeline(PipelineInjector::PipelineType& pipeline)
    {
        for(auto& [type, source]: pipeline)
        {
            std::cout << "START -> type " << type << "\n";
            std::cout << source << "\n";
            std::cout << "END" << std::endl;
        }
    }

    static void printMetadata(const ProgramMetadata& metadata)
    {
        printf("[Metadata] found transformation name: %s\n",metadata.m_TransformationMatrixName.c_str());
        printf("[Metadata] found interface block: %s\n",metadata.m_InterfaceBlockName.c_str());
        printf("[Metadata] is clip space: %d\n",metadata.m_IsClipSpaceTransform);
 
    }
} // helper
namespace {
TEST(PipelineInjector, Insertion) {

    PipelineInjector injector;

    PipelineInjector::PipelineType input =
    {
        {GL_VERTEX_SHADER, R"(
            #version 330
            in vec3 vertex;

            out vec2 normal;

            uniform mat4 insertUniform;
            void main()
            {
                normal = vertex.xy;
                gl_Position = vec4(insertUniform*vertex, 0.0);
            }
        )"},
        {GL_FRAGMENT_SHADER, R"(
            #version 330
            in vec2 normal;
            out vec4 color;
            void main()
            {
                color = vec4(normal, 0.0,0.0);
            }
        )"},

    };

    auto result = injector.process(input);
    helper::printPipeline(result.pipeline);
    if(result.metadata)
        helper::printMetadata(*result.metadata);
}
TEST(PipelineInjector, Injection) {

    PipelineInjector injector;

    PipelineInjector::PipelineType input =
    {
        {GL_VERTEX_SHADER, R"(
            #version 330
            in vec3 vertex;

            out vec2 normal;
            void main()
            {
                normal = vertex.xy;
                gl_Position = vec4(vertex, 0.0);
            }
        )"},
        {GL_GEOMETRY_SHADER, R"(
            #version 330
            in vec2 normal[];
            out vec2 fs_normal;

            layout(triangles) in;
            layout(triangles, max_vertices=3) out;

            void main()
            {
                for(int i = 0; i < 3;i++)
                {
                    gl_Position = gl_in[i].gl_Position;
                    fs_normal = normal[i];
                    EmitVertex();    
                }
            }
        )"},

        {GL_FRAGMENT_SHADER, R"(
            #version 330
            in vec2 fs_normal;
            out vec4 color;
            void main()
            {
                color = vec4(normal, 0.0,0.0);
            }
        )"},

    };

    auto result = injector.process(input);
    helper::printPipeline(result.pipeline);
    if(result.metadata)
        helper::printMetadata(*result.metadata);
}

TEST(PipelineInjector, InOutTest) {

    PipelineInjector injector;

    PipelineInjector::PipelineType input =
    {
        {GL_VERTEX_SHADER, R"(
            #version 330
            in vec3 vertex;

            out vec2 uv;
            
            uniform mat4 mvp;
            void main()
            {
                normal = vertex.xy;
                gl_Position = mvp*vec4(vertex);
            }
        )"},
        {GL_FRAGMENT_SHADER, R"(
            #version 330

            // Wrapper to allow easy sampling for material texture layers
#define HIGH_SAMPLING   4.0
#define MEDIUM_SAMPLING 2.0
#define LOW_SAMPLING    1.0

            vec4 sampleTextureLayer0(vec2 uv)
            {
                return texture(tex_layer_0, uv);
            }


            vec4 multi_sampleTextureLayer3(vec2 uv, float distance)
            {

                vec4 l_col = sampleTextureLayer3(uv * LOW_SAMPLING);
                vec4 m_col = sampleTextureLayer3(uv * MEDIUM_SAMPLING);
                vec4 h_col = sampleTextureLayer3(uv * HIGH_SAMPLING);

                // From Low to medium
                float factor = distance * 0.02;
                factor = pow(factor, 2.5);
                factor = clamp(factor, 0.0, 1.0);
                vec4 f_col = mix(m_col, l_col, factor);
                
                // From medium to high
                factor = distance * 0.1;
                factor = pow(factor, 2.5);
                factor = clamp(factor, 0.0, 1.0);

                f_col = mix(h_col, f_col, factor);

                return f_col;
            }

            vec4 sampleTextureLayer4(vec2 uv)
            {
                return texture(tex_layer_4, uv);
            }

            vec4 multi_sampleTextureLayer4(vec2 uv, float distance)
            {

                vec4 l_col = sampleTextureLayer4(uv * LOW_SAMPLING);
                vec4 m_col = sampleTextureLayer4(uv * MEDIUM_SAMPLING);
                vec4 h_col = sampleTextureLayer4(uv * HIGH_SAMPLING);

                // From Low to medium
                float factor = distance * 0.02;
                factor = pow(factor, 2.5);
                factor = clamp(factor, 0.0, 1.0);
                vec4 f_col = mix(m_col, l_col, factor);
                
                // From medium to high
                factor = distance * 0.1;
                factor = pow(factor, 2.5);
                factor = clamp(factor, 0.0, 1.0);

                f_col = mix(h_col, f_col, factor);

                return f_col;
            }

            vec4 sampleTextureLayer5(vec2 uv)
            {
                return texture(tex_layer_5, uv);
            }


            in vec2 uv;
            out vec4 color;
            void main()
            {
                color = vec4(uv, 0.0,0.0);
            }
        )"},

    };

    auto result = injector.process(input);
    ASSERT_TRUE(result.metadata);
    // All `vec2 uv` should be replaced with corresponding enhancer_frag_ prefix.
    ASSERT_TRUE((result.pipeline[GL_FRAGMENT_SHADER]).find(" uv") == std::string::npos);
}

}
