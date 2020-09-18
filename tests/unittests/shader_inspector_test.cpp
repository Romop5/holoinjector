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
    auto assignments = ve::findAllOutVertexAssignments(shader);
    ASSERT_EQ(assignments.size(), 1);
    ASSERT_EQ(assignments[0].firstTokenFromLeft, "P");
    ASSERT_EQ(ve::getVariableType(assignments[0].firstTokenFromLeft,shader), "mat4");
    ASSERT_EQ(ve::getVariableType("normal",shader), "vec3");
    ASSERT_EQ(ve::getVariableType("MV",shader), "mat4");
}

TEST(ShaderInspector, TextVS) {
    std::string shader = R"(
        layout(location = 3) in vec3 normal;
        int main()
        {
            gl_Position    =   vec4(normal, 1.0);
        }
        )";
    auto assignments = ve::findAllOutVertexAssignments(shader);
    ASSERT_EQ(assignments.size(), 1);
    ASSERT_EQ(assignments[0].firstTokenFromLeft, "vec4");
    ASSERT_TRUE(ve::isBuiltinGLSLType("vec4"));
    ASSERT_TRUE(ve::isBuiltinGLSLType("vec3"));
}

/// Verify that gl_Position is skipped when used as R-value
TEST(ShaderInspector, VSFalseTestMultiline) {
    std::string shader = R"(
        layout(location = 3) in vec3 normal;
        int main()
        {
            gl_Position = vec3(normal, 1.0);
            vec3 test = gl_Position;
            gl_Position = gl_Position+vec3(1.0);
        }
        )";
    auto assignments = ve::findAllOutVertexAssignments(shader);
    ASSERT_EQ(assignments.size(), 2);
    ASSERT_EQ(assignments[0].statementRawText, "gl_Position = vec3(normal, 1.0);");
    ASSERT_EQ(assignments[1].statementRawText, "gl_Position = gl_Position+vec3(1.0);");
}

TEST(ShaderInspector, TextVSXYZ) {
    std::string shader = R"(
        layout(location = 3) in vec3 normal;
        int main()
        {
            gl_Position.xyz    =   vec3(normal, 1.0);
        }
        )";
    auto assignments = ve::findAllOutVertexAssignments(shader);
    ASSERT_EQ(assignments.size(), 0);
}

TEST(ShaderInspector, TextVSReplace) {
    std::string shader = R"(
        layout(location = 3) in vec4 normal;
        int main()
        {
            gl_Position=   vec4(normal, 1.0);
        }
        )";
    auto assignments = ve::findAllOutVertexAssignments(shader);
    ASSERT_EQ(assignments.size(), 1);
    auto result = injectShader(assignments, shader);
    std::cout << "Injected shader: " << result << std::endl;
    // Assert function definition
    ASSERT_NE(result.find("vec4 enhancer_transform"), std::string::npos);
    // Assert function call
    ASSERT_NE(result.find("=enhancer_transform("), std::string::npos);
}
}
