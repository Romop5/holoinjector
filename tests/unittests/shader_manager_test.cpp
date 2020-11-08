#include "gtest/gtest.h"
#include "trackers/shader_manager.hpp"

#include <unordered_set>

using namespace ve;

namespace {
TEST(ShaderManager, Basic) 
{
    ShaderManager m;
    ASSERT_EQ(m.getBoundId(), 0);
    ASSERT_FALSE(m.hasBounded());
    ASSERT_FALSE(m.isVSBound());
    ASSERT_FALSE(m.isGSBound());

    m.add(1,std::make_shared<ve::ShaderProgram>());
    ASSERT_TRUE(m.has(1));

    m.bind(1);
    ASSERT_TRUE(m.hasBounded());
    ASSERT_FALSE(m.isVSBound());

    auto program = m.getBound();
    ASSERT_FALSE(program->shaders.has(GL_VERTEX_SHADER));
    ASSERT_FALSE(program->shaders.has(GL_GEOMETRY_SHADER));
}

TEST(ShaderManager, VertexShaderCreation) 
{
    ShaderManager m;
    m.add(1,std::make_shared<ve::ShaderProgram>());
    m.bind(1);
    ASSERT_EQ(m.getBoundId(), 1);

    auto newTest = std::make_shared<ve::ShaderMetadata>();
    newTest->m_Type = GL_VERTEX_SHADER;
    m.shaders.add(2,newTest);
    m.get(1)->attachShaderToProgram(m.shaders.get(2));

    ASSERT_TRUE(m.hasBounded());
    ASSERT_TRUE(m.isVSBound());
    ASSERT_FALSE(m.isGSBound());
    auto shader = m.getBoundVS();
    ASSERT_EQ(shader->m_Type, GL_VERTEX_SHADER);


    auto program = m.get(1);
    ASSERT_FALSE(program->m_Metadata);
}

TEST(ShaderManager, GeometryShader) 
{
    ShaderManager m;
    m.add(1,std::make_shared<ve::ShaderProgram>());
    m.bind(1);

    auto shader = std::make_shared<ve::ShaderMetadata>();
    shader->m_Type = GL_GEOMETRY_SHADER;
    m.shaders.add(2,shader);
    m.get(1)->attachShaderToProgram(shader);

    ASSERT_TRUE(m.hasBounded());
    ASSERT_FALSE(m.isVSBound());
    ASSERT_TRUE(m.isGSBound());
    shader = m.getBoundGS();
    ASSERT_EQ(shader->m_Type, GL_GEOMETRY_SHADER);


    ASSERT_FALSE(m.get(1)->m_Metadata);
}

TEST(ShaderManager, IsShaderOneOf) 
{
    ShaderManager m;

    std::unordered_set<GLenum> types =
    {
        GL_GEOMETRY_SHADER,
        GL_VERTEX_SHADER,
    };
    size_t shaderID = 1;
    for(const auto& type: types)
    {
        auto newShader = std::make_shared<ve::ShaderMetadata>();
        newShader->m_Type = type;
        m.shaders.add(shaderID,newShader);
        ASSERT_TRUE(m.shaders.has(shaderID));
        auto shader = m.shaders.get(shaderID);
        ASSERT_EQ(shader->m_Type, type);
        ASSERT_TRUE(m.shaders.get(shaderID)->isShaderOneOf(types));
        shaderID++;
    }
}

// Generic shaders (such as fragment shader) are not interesting for us
TEST(ShaderManager, GenericShaderCreation) 
{
    ShaderManager m;
    m.add(1,std::make_shared<ve::ShaderProgram>());
    m.bind(1);

    auto shader = std::make_shared<ve::ShaderMetadata>();
    shader->m_Type = GL_FRAGMENT_SHADER;
    m.shaders.add(2,shader);
    m.get(1)->attachShaderToProgram(shader);

    ASSERT_TRUE(m.hasBounded());
    ASSERT_FALSE(m.isVSBound());
}


TEST(ShaderManager, ProgramEnumeration) 
{
    ShaderManager m;
    m.add(1,std::make_shared<ve::ShaderProgram>());
    ASSERT_EQ(m.getMap().size(),1);
    ASSERT_EQ(m.getMap().size(),1);
    m.add(2,std::make_shared<ve::ShaderProgram>());
    ASSERT_EQ(m.getMap().size(),2);
    ASSERT_EQ(m.getMap().size(),2);

    m.shaders.add(2, std::make_shared<ve::ShaderMetadata>(2, GL_VERTEX_SHADER));
    m.get(2)->attachShaderToProgram(m.shaders.get(2));

    m.bind(2);
    auto shader2 = m.getBound();
    ASSERT_EQ(shader2->shaders.get(GL_VERTEX_SHADER), m.getMap().at(2)->shaders.get(GL_VERTEX_SHADER));
}

}
