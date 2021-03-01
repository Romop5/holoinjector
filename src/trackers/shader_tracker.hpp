#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "utils/context_tracker.hpp"
#include "pipeline/program_metadata.hpp"

#include <GL/gl.h>

namespace ve
{
namespace trackers
{
    struct ShaderMetadata
    {
        ShaderMetadata() = default;
        ShaderMetadata(size_t id, GLenum type)
        {
            m_Id = id;
            m_Type = type;
        }
        size_t m_Id = 0;
        GLenum m_Type = 0;

        std::string preprocessedSourceCode;
        bool isShaderOneOf(const std::unordered_set<GLenum>& allowedTypes);
        const std::string getTypeAsString() const;
    };


    struct ShaderProgram
    {
        BindableContextTracker<std::shared_ptr<ShaderMetadata>> shaders;

        std::unique_ptr<ve::pipeline::ProgramMetadata> m_Metadata;

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


        /* Queries */
        bool hasMetadata() const;
        bool isLinked() const;
    };


    class ShaderTracker: public BindableContextTracker<std::shared_ptr<ShaderProgram>>
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
} // namespace trackers
} // namespace ve

