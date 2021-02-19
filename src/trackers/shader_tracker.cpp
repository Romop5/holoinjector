#include "trackers/shader_tracker.hpp"
#include <algorithm>

#include <cassert>

using namespace ve;
using namespace ve::trackers;

///////////////////////////////////////////////////////////////////////////////
// ShaderMetadata
///////////////////////////////////////////////////////////////////////////////
bool ShaderMetadata::isShaderOneOf(const std::unordered_set<GLenum>& allowedTypes)
{
    return (allowedTypes.count(m_Type) > 0);
}

const std::string ShaderMetadata::ShaderMetadata::getTypeAsString() const
{
    switch(m_Type)
    {
        case GL_VERTEX_SHADER:
            return "VertexShader";
        case GL_FRAGMENT_SHADER:
            return "FragmentShader";
        case GL_GEOMETRY_SHADER:
            return "GeometryShader";
        case GL_COMPUTE_SHADER:
            return "GeometryShader";
        default:
            break;
    }
    return "UknownShader";
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
    shaders.add(shader->m_Type, shader);
    /*
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
    */

    // TODO: remove/move somewhere else
    /*if(shader->m_TransformationMatrixName.empty() || shader->m_InterfaceBlockName.empty())
        return;
    if(m_UniformBlocks.count(shader->m_InterfaceBlockName) > 0)
        return;
    m_UniformBlocks[shader->m_InterfaceBlockName].bindingIndex = 0;
    */
}

///////////////////////////////////////////////////////////////////////////////
// ShaderTracker
///////////////////////////////////////////////////////////////////////////////

bool ShaderTracker::isVSBound() const
{
    if(!hasBounded())
        return false;
    return getBoundConst()->shaders.has(GL_VERTEX_SHADER);
}

bool ShaderTracker::isGSBound() const
{
    if(!hasBounded())
        return false;
    return getBoundConst()->shaders.has(GL_GEOMETRY_SHADER);
}

/// Get metadata for currently bounded program
std::shared_ptr<ShaderMetadata> ShaderTracker::getBoundVS()
{
    assert(hasBounded() != 0);
    return getBound()->shaders.get(GL_VERTEX_SHADER);
}

std::shared_ptr<ShaderMetadata> ShaderTracker::getBoundGS()
{
    assert(hasBounded() != 0);
    return getBound()->shaders.get(GL_GEOMETRY_SHADER);
}
