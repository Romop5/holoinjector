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

const ShaderManager::ShaderProgram& ShaderManager::getProgram(size_t ID) const
{
    return m_programDatabase.at(ID);
}
ShaderManager::ShaderProgram& ShaderManager::getMutableProgram(size_t ID)
{
    return m_programDatabase.at(ID);
}


void ShaderManager::attachShaderToProgram(size_t shaderID, size_t programID)
{
    assert(hasShader(shaderID));
    assert(hasProgram(programID));

    m_programDatabase[programID].m_VertexShader = shaderID; 
}


/// Track bounded programs
void ShaderManager::bind(size_t program)
{
    m_BoundProgram = program;
}

/// Is any valid program bounded
bool ShaderManager::isAnyBound() const
{
    return m_BoundProgram != 0;
}


/// Is VS bound
bool ShaderManager::isVSBound() const
{
    if(!isAnyBound())
        return false;
    const auto VS = getProgram(m_BoundProgram).m_VertexShader;
    return (VS != 1) && hasShader(VS);
}

/// Get metadata for currently bounded program
ShaderManager::ShaderMetadata& ShaderManager::getBoundedVS()
{
    assert(m_BoundProgram != 0);
    const auto VS = getProgram(m_BoundProgram).m_VertexShader;
    assert(VS != -1);
    assert(hasShader(VS));
    return getShaderDescription(VS);
}

ShaderManager::ShaderProgram& ShaderManager::getBoundedProgram()
{
    assert(m_BoundProgram != 0);
    return  getMutableProgram(m_BoundProgram);
}


const std::unordered_map<size_t, ShaderManager::ShaderProgram>& ShaderManager::getPrograms() const
{
    return m_programDatabase;
}

