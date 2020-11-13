#include "trackers/uniform_block_tracing.hpp"

using namespace ve;

bool UniformBlockTracing::hasBufferBindingIndex(size_t buffer) const
{
    return m_UniformBindingMap.count(buffer) > 0;
}
size_t& UniformBlockTracing::getBufferBindingIndex(size_t buffer)
{
    return m_UniformBindingMap.at(buffer);
}
void UniformBlockTracing::setUniformBinding(size_t buffer, size_t bindingIndex)
{
    m_UniformBindingMap[buffer] = bindingIndex;
}

UniformBinding& UniformBlockTracing::getBindingIndex(size_t index)
{
    return m_UniformBindings[index];
}