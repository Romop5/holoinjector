#include <functional>
#include "opengl_redirector_base.hpp"
#include <unordered_map>

#include "glm/glm.hpp"

#include "shader_manager.hpp"

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


        virtual GLuint glCreateProgram (void);


        // Map program to vertex shader
        virtual void glAttachShader (GLuint program, GLuint shader);

        virtual void glDrawArrays(GLenum mode,GLint first,GLsizei count) override;
        virtual void glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) override;
        virtual void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);


        virtual void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);


        virtual int XNextEvent(Display *display, XEvent *event_return);

        private:
        GLint getCurrentProgram();
        void setEnhancerShift(const glm::vec3& clipSpaceTransformation);
        void setEnhancerShift(const glm::mat4& clipSpaceTransformation);

        void duplicateCode(const std::function<void(void)>& code);

        float m_Angle = 0.0;
        float m_Distance = 1.0;

        ShaderManager m_Manager;
    };
}
