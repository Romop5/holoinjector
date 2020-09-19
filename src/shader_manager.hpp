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
        };

        bool hasShader(size_t ID) const;
        void addShader(size_t ID, ShaderTypes type);
        ShaderMetadata& getShaderDescription(size_t ID);

        bool hasProgram(size_t ID) const;
        void addProgram(size_t ID);
        const ShaderProgram& getProgram(size_t ID);

        void attachShaderToProgram(size_t shaderID, size_t programID);

        private:
        /// Maps shader ID to metadata structure
        std::unordered_map<size_t, ShaderMetadata> m_shaderDatabase;

        /// Mapping from program ID to vertex shader ID
        std::unordered_map<size_t, ShaderProgram> m_programDatabase;
    };
} // namespace ve

