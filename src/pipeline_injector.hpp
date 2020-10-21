#ifndef PIPELINE_INJECTOR_HPP
#define PIPELINE_INJECTOR_HPP

#include <unordered_map>
#include <string>
#include <GL/gl.h>

namespace ve
{
    struct GSInjectionParams
    {
        // True when original VS was dividing by 1.0
        bool shouldRenderToClipspace = false;
    };
    class PipelineInjector
    {
        public:
            using PipelineType = std::unordered_map<GLenum, std::string>;
            PipelineType process(PipelineType inputPipeline);
        private:
            /**
             * @brief Inject new geometry shader 
             *
             * @param pipeline
             * @return alterned pipeline with correct in/out attributes passing
             */
            PipelineType injectGeometryShader(const PipelineType& pipeline, const GSInjectionParams params);
    };
} //namespace ve
#endif
