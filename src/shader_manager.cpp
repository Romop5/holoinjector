#include "shader_manager.hpp"

#include <cassert>

using namespace ve;

bool ShaderManager::hasShader(size_t ID) const
{
    return m_shaderDatabase.count(ID) > 0;
}

void ShaderManager::addShader(size_t ID, ShaderTypes type)
{
    ShaderManager::ShaderMetadata meta;
    meta.m_Type = type;
    m_shaderDatabase[ID] = meta;
}

ShaderManager::ShaderMetadata& ShaderManager::getShaderDescription(size_t ID)
{
    assert(hasShader(ID));
    return m_shaderDatabase[ID];
}

bool ShaderManager::hasProgram(size_t ID) const
{
    return m_programDatabase.count(ID) > 0;
}

void ShaderManager::addProgram(size_t ID)
{
    ShaderProgram p;
    m_programDatabase[ID] = p;
}

const ShaderManager::ShaderProgram& ShaderManager::getProgram(size_t ID)
{
    return m_programDatabase[ID];
}

void ShaderManager::attachShaderToProgram(size_t shaderID, size_t programID)
{
    assert(hasShader(shaderID));
    assert(hasProgram(programID));

    m_programDatabase[programID].m_VertexShader = shaderID; 
}

