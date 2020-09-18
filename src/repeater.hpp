#include <functional>
#include "opengl_redirector_base.hpp"
#include <unordered_map>

#include "glm/glm.hpp"

namespace ve
{
    struct Viewport
    {
        GLint size[4];
    };

    class Repeater: public OpenglRedirectorBase
    {
        public:
        virtual void registerCallbacks() override;

        virtual void glClear(GLbitfield mask) override;

        virtual GLuint glCreateShader(GLenum shaderType);
        virtual void glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length);


        // Map program to vertex shader
        virtual void glAttachShader (GLuint program, GLuint shader);

        virtual void glDrawArrays(GLenum mode,GLint first,GLsizei count) override;
        virtual void glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) override;


        virtual void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);


        virtual int XNextEvent(Display *display, XEvent *event_return);

        private:
        GLint getCurrentProgram();
        void setEnhancerShift(const glm::vec3& clipSpaceTransformation);
        void setEnhancerShift(const glm::mat4& clipSpaceTransformation);

        void duplicateCode(const std::function<void(void)>& code);

        struct ShaderMetadata
        {
            bool isVertexShader;
            std::string transformationMatrixName;
        };

        /// Maps shader ID to metadata structure
        std::unordered_map<size_t, ShaderMetadata> m_shaderDatabase;

        /// Mapping from program ID to vertex shader ID
        std::unordered_map<size_t, size_t> m_programVertexShaderDatabase;

        float angle = 0.0;
    };
}
