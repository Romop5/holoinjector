#include <functional>
#include "opengl_redirector_base.hpp"
#include <unordered_map>

#include "glm/glm.hpp"

#include "shader_manager.hpp"
#include "framebuffer_tracker.hpp"

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

        // Shader start
        virtual GLuint glCreateShader(GLenum shaderType);
        virtual void glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length);

        // Map program to vertex shader
        virtual void glAttachShader (GLuint program, GLuint shader);
        // Shader end 

        virtual GLuint glCreateProgram (void);
        virtual void glUseProgram (GLuint program);




        // Framebuffers
        virtual void glGenFramebuffers (GLsizei n, GLuint* framebuffers);
        virtual void glBindFramebuffer (GLenum target, GLuint framebuffer);
        virtual void glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level);
        virtual void glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
        virtual void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
        virtual void glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
        // Framebuffers end

        // Draw calls start
        virtual void glDrawArrays(GLenum mode,GLint first,GLsizei count) override;
        virtual void glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) override;
        virtual void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
        // Draw calls end 
        virtual void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);



        virtual int XNextEvent(Display *display, XEvent *event_return);

        private:
        GLint getCurrentProgram();
        void setEnhancerShift(const glm::vec3& clipSpaceTransformation);
        void setEnhancerShift(const glm::mat4& clipSpaceTransformation);
        void setEnhancerIdentity();

        void duplicateCode(const std::function<void(void)>& code);

        float m_Angle = 0.0;
        float m_Distance = 1.0;

        bool m_IsDuplicationOn = false;

        ShaderManager m_Manager;
        FramebufferTracker m_FBOTracker;
    };
}
