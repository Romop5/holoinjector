#include "gtest/gtest.h"
#include "shader_inspector.hpp"

namespace {
TEST(ShaderInspector, Basics) {
    /* A typical vertex shader*/
    std::string shader = R"(
        uniform mat4 MV ;
        uniform vec3 normal;
        uniform mat4 MVP  ;
        uniform mat4    P   ;
        int main()
        {
            gl_Position    =   P*   MVP*vec4(1.0);
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 1);
    ASSERT_EQ(assignments[0].firstTokenFromLeft, "P");
    ASSERT_EQ(inspector.getVariableType(assignments[0].firstTokenFromLeft), "mat4");
    ASSERT_EQ(inspector.getVariableType("normal"), "vec3");
    ASSERT_EQ(inspector.getVariableType("MV"), "mat4");
    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "P");
}

TEST(ShaderInspector, TextVS) {
    std::string shader = R"(
        layout(location = 3) in vec3 normal;
        int main()
        {
            gl_Position    =   vec4(normal, 1.0);
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 1);
    ASSERT_EQ(assignments[0].firstTokenFromLeft, "vec4");
    ASSERT_TRUE(inspector.isBuiltinGLSLType("vec4"));
    ASSERT_TRUE(inspector.isBuiltinGLSLType("vec3"));
    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "");
}

/// Verify that gl_Position is skipped when used as R-value
TEST(ShaderInspector, VSFalseTestMultiline) {
    std::string shader = R"(
        #version 330 core
        layout(location = 3) in vec3 normal;
        int main()
        {
            gl_Position = vec3(normal, 1.0);
            vec3 test = gl_Position;
            gl_Position = gl_Position+vec3(1.0);
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 2);
    ASSERT_EQ(assignments[0].statementRawText, "gl_Position = vec3(normal, 1.0);");
    ASSERT_EQ(assignments[1].statementRawText, "gl_Position = gl_Position+vec3(1.0);");

    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "");
    ASSERT_EQ(inspector.getCountOfUniforms(), 0);
}

TEST(ShaderInspector, TextVSXYZ) {
    std::string shader = R"(
        #version 330 core
        layout(location = 3) in vec3 normal;
        int main()
        {
            gl_Position.xyz    =   vec3(normal, 1.0);
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 0);
    ASSERT_EQ(inspector.getCountOfUniforms(), 0);
}

TEST(ShaderInspector, TextVSReplace) {
    std::string shader = R"(
        #version 330 core
        layout(location = 3) in vec4 normal;
        int main()
        {
            gl_Position=   vec4(normal, 1.0);
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 1);
    auto result = inspector.injectShader(assignments);
    std::cout << "Injected shader: " << result << std::endl;
    // Assert function definition
    ASSERT_NE(result.find("vec4 enhancer_transform"), std::string::npos);
    // Assert function call
    ASSERT_NE(result.find("=enhancer_transform_HUD("), std::string::npos);

    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "");
    ASSERT_EQ(inspector.getCountOfUniforms(), 0);
}

TEST(ShaderInspector, VSTakeFirstMVP) {
    std::string shader = R"(
        #version 330 core
        layout(location = 0) in vec4 position;
        uniform mat4 MV;
        uniform mat4 MVP;
        int main()
        {
            gl_Position   =   MVP*position;
            // Some arbitrary constant offset => just for test
            gl_Position = gl_Position + vec4(0.0,0.0,1.0,0.0);
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 2);
    ASSERT_EQ(assignments[0].statementRawText, "gl_Position   =   MVP*position;");
    ASSERT_EQ(assignments[1].statementRawText, "gl_Position = gl_Position + vec4(0.0,0.0,1.0,0.0);");

    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "MVP");
    ASSERT_EQ(inspector.getCountOfUniforms(), 2);
}

TEST(ShaderInspector, VSExample) {
    std::string shader = R"(
        #version 330 core

        // Input vertex data, different for all executions of this shader.
        layout(location = 0) in vec3 vertexPosition_modelspace;
        layout(location = 1) in vec2 vertexUV;

        // Output data ; will be interpolated for each fragment.
        out vec2 UV;

        // Values that stay constant for the whole mesh.
        uniform mat4 MVP;

        void main(){

                // Output position of the vertex, in clip space : MVP * position
                gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
                
                // UV of the vertex. No special space for this one.
                UV = vertexUV;
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 1);
    ASSERT_EQ(assignments[0].statementRawText, "gl_Position =  MVP * vec4(vertexPosition_modelspace,1);");

    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "MVP");
    ASSERT_EQ(inspector.getCountOfUniforms(), 1);
}

TEST(ShaderInspector, InterfaceBlockUniform) {
    std::string shader = R"(
        #version 330 core

        // Input vertex data, different for all executions of this shader.
        layout(location = 0) in vec3 vertexPosition_modelspace;
        layout(location = 1) in vec2 vertexUV;

        // Output data ; will be interpolated for each fragment.
        out vec2 UV;

        // Values that stay constant for the whole mesh.
        uniform Hello{ 
            mat4 MVP;
        };

        void main(){

                // Output position of the vertex, in clip space : MVP * position
                gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
                
                // UV of the vertex. No special space for this one.
                UV = vertexUV;
        }
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 1);
    ASSERT_EQ(assignments[0].statementRawText, "gl_Position =  MVP * vec4(vertexPosition_modelspace,1);");

    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "MVP");
    ASSERT_EQ(inspector.getCountOfUniforms(), 1);
}

TEST(ShaderInspector, VSClipSpace) {
    std::string shader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        out vec3 TexCoords;

        uniform mat4 projection;
        uniform mat4 view;

        void main()
        {
            TexCoords = aPos;
            vec4 pos = projection * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        }        
        )";
    auto inspector = ve::ShaderInspector(shader);
    auto assignments = inspector.findAllOutVertexAssignments();
    ASSERT_EQ(assignments.size(), 1);
    ASSERT_EQ(assignments[0].statementRawText, "gl_Position = pos.xyww;");

    ASSERT_EQ(inspector.getTransformationUniformName(assignments), "");
    ASSERT_EQ(inspector.getCountOfUniforms(), 2);
}

TEST(ShaderInspector, VSUniforms) {
    std::string shader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        out vec3 TexCoords;

        uniform mat4 projection;
        uniform mat4 view;

        void main()
        {
            TexCoords = aPos;
            vec4 pos = projection * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        }        
        )";
    auto inspector = ve::ShaderInspector(shader);
    inspector.getListOfUniforms();
}
}
