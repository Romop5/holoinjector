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
            size_t m_VertexShader;
        };
        struct ShaderMetadata
        {
            ShaderTypes m_Type;
            std::string m_TransformationMatrixName;
            // Determines if any uniform is defined
            bool m_HasAnyUniform = false;
            // Determines if skybox / clipspace rendering was detected
            bool m_IsClipSpaceTransform = false;
        };

        bool hasShader(size_t ID) const;
        void addShader(size_t ID, ShaderTypes type);
        ShaderMetadata& getShaderDescription(size_t ID);

        bool hasProgram(size_t ID) const;
        void addProgram(size_t ID);
        const ShaderProgram& getProgram(size_t ID) const;

        void attachShaderToProgram(size_t shaderID, size_t programID);

        /// Track bounded programs
        void bind(size_t program);
        
        /// Is any valid program bounded
        bool isAnyBound() const;

        /// Is VS bound
        bool isVSBound() const;

        /// Get metadata for currently bounded program
        ShaderMetadata& getBoundedVS();

        private:
        /// Maps shader ID to metadata structure
        std::unordered_map<size_t, ShaderMetadata> m_shaderDatabase;

        /// Mapping from program ID to vertex shader ID
        std::unordered_map<size_t, ShaderProgram> m_programDatabase;

        /// last glUseProgram
        size_t m_BoundProgram;
    };
} // namespace ve

