#ifndef VE_PROGRAM_METADATA_HPP
#define VE_PROGRAM_METADATA_HPP

#include <string>

namespace ve 
{
namespace pipeline
{
struct ProgramMetadata
{
    bool m_IsUniformInInterfaceBlock = false;
    /// If transformation is in block, stores block name
    std::string m_InterfaceBlockName;

    /// Name of projection matrix or MVP
    std::string m_TransformationMatrixName;
    // Determines if any uniform is defined
    bool m_HasAnyUniform = false;
    // Determines if skybox / clipspace rendering was detected
    bool m_IsClipSpaceTransform = false;


    // Queries
    bool isUBOused() const;
    bool hasDetectedTransformation() const;
};

} //namespace pipeline
} //namespace ve
#endif
