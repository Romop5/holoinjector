#include <string>
#include <unordered_map>

namespace ve
{
    class ShaderManager
    {
        public:
        enum ShaderTypes
        {
            GENERIC,
            VS,
        };

        struct ShaderProgram
        {
            size_t m_VertexShader = -1;

            struct UniformBlock
            {
                size_t location = -1;
                size_t bindingIndex = -1;
            };
            std::unordered_map<std::string, UniformBlock> m_UniformBlocks;
        };
        struct ShaderMetadata
        {
            ShaderTypes m_Type;

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
            bool isUBOused() const
            {
                return !m_TransformationMatrixName.empty() && !m_InterfaceBlockName.empty();
            }
        };

        bool hasShader(size_t ID) const;
        void addShader(size_t ID, ShaderTypes type);
        ShaderMetadata& getShaderDescription(size_t ID);

        bool hasProgram(size_t ID) const;
        void addProgram(size_t ID);
        const ShaderProgram& getProgram(size_t ID) const;
        ShaderProgram& getMutableProgram(size_t ID);

        void attachShaderToProgram(size_t shaderID, size_t programID);

        /// Track bounded programs
        void bind(size_t program);
        
        /// Is any valid program bounded
        bool isAnyBound() const;

        /// Is VS bound
        bool isVSBound() const;

        /// Get metadata for currently bounded program
        ShaderMetadata& getBoundedVS();

        /// Get metadata for currently bounded program
        ShaderProgram& getBoundedProgram();

        const std::unordered_map<size_t, ShaderProgram>& getPrograms() const;
        private:
        /// Maps shader ID to metadata structure
        std::unordered_map<size_t, ShaderMetadata> m_shaderDatabase;

        /// Mapping from program ID to vertex shader ID
        std::unordered_map<size_t, ShaderProgram> m_programDatabase;

        /// last glUseProgram
        size_t m_BoundProgram;
    };
} // namespace ve

