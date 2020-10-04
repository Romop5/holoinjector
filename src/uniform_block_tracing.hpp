#include <glm/glm.hpp>
#include <unordered_map>
namespace ve
{
    /// Should be filled by shader program during glUniformBlockBinding
    struct UniformBinding
    {
        bool hasTransformation = false;
        size_t transformationOffset = -1;
        glm::mat4 transformation;
        bool isOrthogonal;
        glm::vec4 decodedParams;
    };

    class UniformBlockTracing
    {
        public:
        bool hasBufferBindingIndex(size_t buffer) const
        {
            return m_UniformBindingMap.count(buffer) > 0;
        }
        size_t& getBufferBindingIndex(size_t buffer)
        {
            return m_UniformBindingMap.at(buffer);
        }
        void setUniformBinding(size_t buffer, size_t bindingIndex)
        {
            m_UniformBindingMap[buffer] = bindingIndex;
        }

        UniformBinding& getBindingIndex(size_t index)
        {
            return m_UniformBindings[index];
        }
        private:
            // buffer object to uniform binding index
            std::unordered_map<size_t, size_t> m_UniformBindingMap;

            // binding point (e.g. 0) to metadata
            std::unordered_map<size_t, UniformBinding> m_UniformBindings;
    };
} //namespace ve
