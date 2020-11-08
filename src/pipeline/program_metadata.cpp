#include "pipeline/program_metadata.hpp"

using namespace ve;

bool ProgramMetadata::isUBOused() const
{
    return !m_TransformationMatrixName.empty() && !m_InterfaceBlockName.empty();
}

bool ProgramMetadata::hasDetectedTransformation() const
{
    return !m_TransformationMatrixName.empty();
}


