#include "gtest/gtest.h"
#include "pipeline_injector.hpp"
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
} // helper
namespace {
TEST(PipelineInjector, Basic) {

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
    helper::printPipeline(result);
}
}
