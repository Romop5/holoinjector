#ifndef PIPELINE_INJECTOR_HPP
#define PIPELINE_INJECTOR_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include <GL/gl.h>

#include "pipeline/program_metadata.hpp"

namespace ve
{
    struct PipelineParams
    {
        // True when original VS was dividing by 1.0
        bool shouldRenderToClipspace = false;

        size_t countOfPrimitivesDuplicates = 1;
        size_t countOfInvocations = 1;
    };

    class PipelineInjector
    {
        public:
            using PipelineType = std::unordered_map<GLenum, std::string>;
            struct PipelineProcessResult
            {
                PipelineType pipeline;
                std::unique_ptr<ProgramMetadata> metadata;
            };
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


            bool injectShader(std::string& sourceCode, ProgramMetadata& outMetadata);
    };
} //namespace ve
#endif
