#include "shader_manager.hpp"
#include <algorithm>

#include <cassert>

using namespace ve;

///////////////////////////////////////////////////////////////////////////////
// ShaderMetadata
///////////////////////////////////////////////////////////////////////////////
bool ShaderMetadata::isShaderOneOf(const std::unordered_set<GLenum>& allowedTypes)
{
    return (allowedTypes.count(m_Type) > 0);
}


bool ShaderMetadata::isUBOused() const
{
    return !m_TransformationMatrixName.empty() && !m_InterfaceBlockName.empty();
}

bool ShaderMetadata::hasDetectedTransformation() const
{
    return !m_TransformationMatrixName.empty();
}


///////////////////////////////////////////////////////////////////////////////
// ShaderProgram
///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::updateUniformBlock(size_t location, size_t bindingIndex)
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

bool ShaderProgram::hasUniformBlock(const std::string& name) const
{
    return m_UniformBlocks.count(name) > 0;
}

void ShaderProgram::attachShaderToProgram(std::shared_ptr<ShaderMetadata> shader)
{

    switch(shader->m_Type)
    {
        case GL_GEOMETRY_SHADER:
            m_GeometryShader = shader; 
            break;
        case GL_VERTEX_SHADER:
            m_VertexShader = shader; 
            break;
        default:
            break;
    }

    // TODO: remove/move somewhere else
    if(shader->m_TransformationMatrixName.empty() || shader->m_InterfaceBlockName.empty())
        return;
    if(m_UniformBlocks.count(shader->m_InterfaceBlockName) > 0)
        return;
    m_UniformBlocks[shader->m_InterfaceBlockName].bindingIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////
// ShaderManager
///////////////////////////////////////////////////////////////////////////////

bool ShaderManager::isVSBound() const
{
    if(!hasBounded())
        return false;
    const auto& VS = getBoundConst()->m_VertexShader;
    return VS != nullptr;
}

bool ShaderManager::isGSBound() const
{
    if(!hasBounded())
        return false;
    const auto& GS = getBoundConst()->m_GeometryShader;
    return GS != nullptr;
}

/// Get metadata for currently bounded program
std::shared_ptr<ShaderMetadata> ShaderManager::getBoundVS()
{
    assert(hasBounded() != 0);
    const auto& VS = getBound()->m_VertexShader;
    return VS;
}

std::shared_ptr<ShaderMetadata> ShaderManager::getBoundGS()
{
    assert(hasBounded() != 0);
    const auto GS = getBound()->m_GeometryShader;
    return GS;
}
