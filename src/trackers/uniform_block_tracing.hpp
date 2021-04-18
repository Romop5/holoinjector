/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        trackers/uniform_block_tracing.hpp
*
*****************************************************************************/

#ifndef HI_UNIFORM_BLOCK_TRACING_HPP
#define HI_UNIFORM_BLOCK_TRACING_HPP
#include "pipeline/projection_estimator.hpp"
#include <glm/glm.hpp>
#include <unordered_map>

namespace hi
{
namespace trackers
{
    /// Should be filled by shader program during glUniformBlockBinding
    struct UniformBinding
    {
        bool hasTransformation = false;
        size_t transformationOffset = -1;
        glm::mat4 transformation;
        hi::pipeline::PerspectiveProjectionParameters projection;
    };

    /**
     *
     */
    class UniformBlockTracing
    {
    public:
        bool hasBufferBindingIndex(size_t buffer) const;
        size_t& getBufferBindingIndex(size_t buffer);
        void setUniformBinding(size_t buffer, size_t bindingIndex);

        UniformBinding& getBindingIndex(size_t index);

    private:
        // buffer object to uniform binding index
        std::unordered_map<size_t, size_t> m_UniformBindingMap;

        // binding point (e.g. 0) to metadata
        std::unordered_map<size_t, UniformBinding> m_UniformBindings;
    };
} //namespace trackers
} //namespace hi
#endif
