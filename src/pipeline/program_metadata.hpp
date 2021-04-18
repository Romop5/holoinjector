/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/program_metadata.hpp
*
*****************************************************************************/

#ifndef HI_PROGRAM_METADATA_HPP
#define HI_PROGRAM_METADATA_HPP

#include <string>

namespace hi
{
namespace pipeline
{
    struct ProgramMetadata
    {
        bool m_IsInvisible = false;
        bool m_IsInjected = false;
        bool m_IsUniformInInterfaceBlock = false;
        /// If transformation is in block, stores block name
        std::string m_InterfaceBlockName;

        /// Name of projection matrix or MVP
        std::string m_TransformationMatrixName;
        // Determines if any uniform is defined
        bool m_HasAnyUniform = false;
        // Determines if skybox / clipspace rendering was detected
        bool m_IsClipSpaceTransform = false;
        // Determines if ftransform() is called in Vertex Shader
        bool m_HasAnyFtransform = false;

        // Is geometry shader user
        bool m_IsGeometryShaderUsed = true;

        // Is linked by enhancer correctly
        bool m_IsLinkedCorrectly = false;
        // Queries
        bool isUBOused() const;
        bool hasDetectedTransformation() const;
        bool hasFtransform() const;
        bool usesGeometryShader() const;
        bool isLinked() const;
    };

} //namespace pipeline
} //namespace hi
#endif
