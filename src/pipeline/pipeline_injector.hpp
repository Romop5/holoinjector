/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/pipeline_injector.hpp
*
*****************************************************************************/

#ifndef HI_PIPELINE_INJECTOR_HPP
#define HI_PIPELINE_INJECTOR_HPP

#include <GL/gl.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "pipeline/program_metadata.hpp"
#include "pipeline/shader_profile.hpp"

namespace hi
{
namespace pipeline
{
    struct PipelineParams
    {
        // True when original VS was dividing by 1.0
        bool shouldRenderToClipspace = false;

        // When true, inject VertexShader instead of inserting Geometry Shader
        bool shouldPreventGeometryShaderInsertion = false;

        size_t countOfPrimitivesDuplicates = 1;
        size_t countOfInvocations = 9;
    };

    /*
     * @brief Alterns original shaders to achieve multiple views
     *
     * PipelineInjector is used to centrally defined processing of shaders.
     * The PipelineInjector can:
     * - insert Geometry shader to force multiple invocations for multi-layered
     *   rendering (@see insertGeometryShader)
     * - inject application's Geometry shader
     * - inspect both GS & Vertex Shader's transformation (@see process)
     */
    class PipelineInjector
    {
    public:
        explicit PipelineInjector(ShaderProfile& profileInst);
        using PipelineType = std::unordered_map<GLenum, std::string>;
        struct PipelineProcessResult
        {
            bool wasSuccessfull;
            PipelineType pipeline;
            std::unique_ptr<ProgramMetadata> metadata;
            std::optional<std::string> failReason;
        };

        /// Create non-modified pipeline
        PipelineProcessResult identity(PipelineType inputPipeline);

        /**
             * @brief
             *
             * @param inputPipeline
             *
             * @return 
             */
        PipelineProcessResult process(PipelineType inputPipeline, const PipelineParams& params = PipelineParams());

    private:
        /**
             * @brief Insert new geometry shader
             *
             * @param pipeline
             * @return alterned pipeline with correct in/out attributes passing
             */
        PipelineType insertGeometryShader(const PipelineType& pipeline, const PipelineParams params);
        /**
             * @brief Inject the old GS
             *
             * @param pipeline
             * @param params
             *
             * @return
             */
        PipelineType injectGeometryShader(const PipelineType& pipeline, const PipelineParams params);

        /// Insert repeating logic into Vertex shader and dont use any additional GS
        PipelineType injectVertexShader(const PipelineType& pipeline, const PipelineParams params);

        bool injectShader(std::string& sourceCode, ProgramMetadata& outMetadata);

        ShaderProfile& profiles;
    };
} //namespace pipeline
} //namespace hi
#endif
