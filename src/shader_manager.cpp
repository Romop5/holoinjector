#include "shader_manager.hpp"
#include <algorithm>

#include <cassert>

using namespace ve;


void ShaderManager::ShaderProgram::updateUniformBlock(size_t location, size_t bindingIndex)
{
    auto blockReference = std::find_if(m_UniformBlocks.begin(), m_UniformBlocks.end(),[&](const auto& block)->bool
    {
        return block.second.location == location;    
    });
    if(blockReference != m_UniformBlocks.end())
    {
        (*blockReference).second.bindingIndex = bindingIndex;
    }
}

bool ShaderManager::ShaderProgram::hasUniformBlock(const std::string& name) const
{
    return m_UniformBlocks.count(name) > 0;
}

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

bool ShaderManager::isShaderOneOf(size_t ID, const std::unordered_set<ShaderTypes>& allowedTypes)
{
    auto& meta = getShaderDescription(ID);
    return (allowedTypes.count(meta.m_Type) > 0);
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


void ShaderManager::attachShaderToProgram(ShaderTypes type, size_t shaderID, size_t programID)
{
    assert(hasShader(shaderID));
    assert(hasProgram(programID));

    auto& program = getMutableProgram(programID);
    switch(type)
    {
        case GEOMETRY:
            program.m_GeometryShader = shaderID; 
            break;
        case VS:
            program.m_VertexShader = shaderID; 
            break;
        default:
            break;
    }

    auto& description = getShaderDescription(shaderID);
    if(description.m_TransformationMatrixName.empty() || description.m_InterfaceBlockName.empty())
        return;
    if(program.m_UniformBlocks.count(description.m_InterfaceBlockName) > 0)
        return;
    program.m_UniformBlocks[description.m_InterfaceBlockName].bindingIndex = 0;
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
    return (VS != -1) && hasShader(VS);
}

bool ShaderManager::isGSBound() const
{
    if(!isAnyBound())
        return false;
    const auto GS = getProgram(m_BoundProgram).m_GeometryShader;
    return (GS != -1) && hasShader(GS);
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

ShaderManager::ShaderMetadata& ShaderManager::getBoundedGS()
{
    assert(m_BoundProgram != 0);
    const auto GS = getProgram(m_BoundProgram).m_GeometryShader;
    assert(GS != -1);
    assert(hasShader(GS));
    return getShaderDescription(GS);
}


ShaderManager::ShaderProgram& ShaderManager::getBoundedProgram()
{
    assert(m_BoundProgram != 0);
    return  getMutableProgram(m_BoundProgram);
}

size_t ShaderManager::getBoundedProgramID() const
{
    return m_BoundProgram;
}

const std::unordered_map<size_t, ShaderManager::ShaderProgram>& ShaderManager::getPrograms() const
{
    return m_programDatabase;
}

std::unordered_map<size_t, ShaderManager::ShaderProgram>& ShaderManager::getMutablePrograms() 
{
    return m_programDatabase;
}


