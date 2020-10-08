#include "gtest/gtest.h"
#include "shader_manager.hpp"

#include <unordered_set>

using namespace ve;

namespace {
TEST(ShaderManager, Basic) 
{
    ShaderManager m;
    ASSERT_EQ(m.getBoundedProgramID(), 0);
    ASSERT_FALSE(m.isAnyBound());
    ASSERT_FALSE(m.isVSBound());
    ASSERT_FALSE(m.isGSBound());

    m.addProgram(1);
    ASSERT_TRUE(m.hasProgram(1));

    m.bind(1);
    ASSERT_TRUE(m.isAnyBound());
    ASSERT_FALSE(m.isVSBound());

    auto& program = m.getBoundedProgram();
    ASSERT_EQ(program.m_VertexShader, -1);
    ASSERT_EQ(program.m_GeometryShader, -1);
}

TEST(ShaderManager, VertexShaderCreation) 
{
    ShaderManager m;
    m.addProgram(1);
    m.bind(1);
    ASSERT_EQ(m.getBoundedProgramID(), 1);

    m.addShader(2,ve::ShaderManager::ShaderTypes::VS);
    m.attachShaderToProgram(ve::ShaderManager::ShaderTypes::VS, 2,1);

    ASSERT_TRUE(m.isAnyBound());
    ASSERT_TRUE(m.isVSBound());
    ASSERT_FALSE(m.isGSBound());
    auto& shader = m.getBoundedVS();
    ASSERT_EQ(shader.m_Type, ve::ShaderManager::ShaderTypes::VS);
    ASSERT_FALSE(shader.hasDetectedTransformation());
    ASSERT_FALSE(shader.isUBOused());
}

TEST(ShaderManager, GeometryShader) 
{
    ShaderManager m;
    m.addProgram(1);
    m.bind(1);

    m.addShader(2,ve::ShaderManager::ShaderTypes::GEOMETRY);
    m.attachShaderToProgram(ve::ShaderManager::ShaderTypes::GEOMETRY, 2,1);

    ASSERT_TRUE(m.isAnyBound());
    ASSERT_FALSE(m.isVSBound());
    ASSERT_TRUE(m.isGSBound());
    auto& shader = m.getBoundedGS();
    ASSERT_EQ(shader.m_Type, ve::ShaderManager::ShaderTypes::GEOMETRY);
    ASSERT_FALSE(shader.isUBOused());
}

TEST(ShaderManager, IsShaderOneOf) 
{
    ShaderManager m;

    std::unordered_set<ShaderManager::ShaderTypes> types =
    {
        ShaderManager::ShaderTypes::GEOMETRY,
        ShaderManager::ShaderTypes::VS,
    };
    size_t shaderID = 1;
    for(const auto& type: types)
    {
        m.addShader(shaderID,type);
        ASSERT_TRUE(m.hasShader(shaderID));
        auto& shader = m.getShaderDescription(shaderID);
        ASSERT_EQ(shader.m_Type, type);
        ASSERT_TRUE(m.isShaderOneOf(shaderID, types));
        shaderID++;
    }
}

// Generic shaders (such as fragment shader) are not interesting for us
TEST(ShaderManager, GenericShaderCreation) 
{
    ShaderManager m;
    m.addProgram(1);
    m.bind(1);

    m.addShader(2,ve::ShaderManager::ShaderTypes::GENERIC);
    m.attachShaderToProgram(ve::ShaderManager::ShaderTypes::GENERIC, 2,1);

    ASSERT_TRUE(m.isAnyBound());
    ASSERT_FALSE(m.isVSBound());
}


TEST(ShaderManager, ProgramEnumeration) 
{
    ShaderManager m;
    m.addProgram(1);
    ASSERT_EQ(m.getPrograms().size(),1);
    ASSERT_EQ(m.getMutablePrograms().size(),1);
    m.addProgram(2);
    ASSERT_EQ(m.getPrograms().size(),2);
    ASSERT_EQ(m.getMutablePrograms().size(),2);

    m.addShader(2, ve::ShaderManager::ShaderTypes::VS);
    m.attachShaderToProgram(ve::ShaderManager::ShaderTypes::VS, 2, 2);

    m.bind(2);
    auto& shader2 = m.getBoundedProgram();
    ASSERT_EQ(shader2.m_VertexShader, m.getPrograms().at(2).m_VertexShader);
}

}
