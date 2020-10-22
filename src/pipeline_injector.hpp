#ifndef PIPELINE_INJECTOR_HPP
#define PIPELINE_INJECTOR_HPP

#include <unordered_map>
#include <string>
#include <GL/gl.h>

namespace ve
{
    struct GSInsertionParams
    {
        // True when original VS was dividing by 1.0
        bool shouldRenderToClipspace = false;
    };

    struct GSInjectionParams
    {
        size_t countOfPrimitivesDuplicates = 1;
        size_t countOfInvocations = 1;
    };

    class PipelineInjector
    {
        public:
            using PipelineType = std::unordered_map<GLenum, std::string>;
            PipelineType process(PipelineType inputPipeline);
        private:
            /**
             * @brief Insert new geometry shader 
             *
             * @param pipeline
             * @return alterned pipeline with correct in/out attributes passing
             */
            PipelineType insertGeometryShader(const PipelineType& pipeline, const GSInsertionParams params);
            /**
             * @brief Inject the old GS
             *
             * @param pipeline
             * @param params
             *
             * @return 
             */
            PipelineType injectGeometryShader(const PipelineType& pipeline, const GSInjectionParams params);
    };
} //namespace ve
#endif
