#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "utils/context_tracker.hpp"

#include <GL/gl.h>

namespace ve
{
    struct ShaderMetadata
    {
        GLenum m_Type = 0;

        std::string preprocessedSourceCode;

        bool m_IsUniformInInterfaceBlock = false;
        /// If transformation is in block, stores block name
        std::string m_InterfaceBlockName;
        /// Offset from the start of block
        size_t m_TransformationByteOffset;

        /// Name of projection matrix or MVP
        std::string m_TransformationMatrixName;
        // Determines if any uniform is defined
        bool m_HasAnyUniform = false;
        // Determines if skybox / clipspace rendering was detected
        bool m_IsClipSpaceTransform = false;


        // Queries
        bool isUBOused() const;
        bool hasDetectedTransformation() const;
        bool isShaderOneOf(const std::unordered_set<GLenum>& allowedTypes);
    };


    struct ShaderProgram
    {
        std::shared_ptr<ShaderMetadata> m_VertexShader;
        std::shared_ptr<ShaderMetadata> m_GeometryShader;

        struct UniformBlock
        {
            size_t location = -1;
            // by default, each uniform block is binded to 0
            size_t bindingIndex = 0;
        };
        std::unordered_map<std::string, UniformBlock> m_UniformBlocks;

        /// Set binding index of Uniform Block with location
        void updateUniformBlock(size_t location, size_t bindingIndex);
        bool hasUniformBlock(const std::string& name) const;

        void attachShaderToProgram(std::shared_ptr<ShaderMetadata> shader);
    };


    class ShaderManager: public BindableContextTracker<std::shared_ptr<ShaderProgram>>
    {
        public:
        ContextTracker<std::shared_ptr<ShaderMetadata>> shaders;

        /// Is VS bound
        bool isVSBound() const;

        /// Is GS bound
        bool isGSBound() const;

        /// Get metadata for currently bounded program
        std::shared_ptr<ShaderMetadata> getBoundVS();

        /// Get metadata for currently bounded program
        std::shared_ptr<ShaderMetadata> getBoundGS();
    };
} // namespace ve

