/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/program_metadata.cpp
*
*****************************************************************************/

#include "pipeline/program_metadata.hpp"

using namespace hi;
using namespace hi::pipeline;

bool ProgramMetadata::isUBOused() const
{
    return !m_TransformationMatrixName.empty() && !m_InterfaceBlockName.empty();
}

bool ProgramMetadata::hasDetectedTransformation() const
{
    return !m_TransformationMatrixName.empty();
}

bool ProgramMetadata::hasFtransform() const
{
    return m_HasAnyFtransform;
}

bool ProgramMetadata::usesGeometryShader() const
{
    return m_IsGeometryShaderUsed;
}

bool ProgramMetadata::isLinked() const
{
    return m_IsLinkedCorrectly;
}
