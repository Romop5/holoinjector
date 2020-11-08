#include "gtest/gtest.h"
#include "pipeline/pipeline_injector.hpp"
using namespace ve;
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
}
