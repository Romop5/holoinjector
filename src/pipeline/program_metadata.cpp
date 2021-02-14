#include "pipeline/program_metadata.hpp"

using namespace ve;
using namespace ve::pipeline;

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


