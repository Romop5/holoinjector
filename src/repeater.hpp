#include <functional>
#include "opengl_redirector_base.hpp"
#include <unordered_map>

#include <GL/gl.h>
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

        // Swap buffers
        virtual void glXSwapBuffers(	Display * dpy, GLXDrawable drawable);

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
        virtual void glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices);

        // Draw calls end 
        virtual void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);


        // Viewport start
        virtual void glViewport(GLint x,GLint y,GLsizei width,GLsizei height);
        virtual void glScissor(GLint x,GLint y,GLsizei width,GLsizei height);
        // Viewport end 
        
        virtual int XNextEvent(Display *display, XEvent *event_return);

        private:
        GLint getCurrentProgram();
        void setEnhancerShift(const glm::mat4& clipSpaceTransformation);
        void resetEnhancerShift();
        void setEnhancerIdentity();

        void duplicateCode(const std::function<void(void)>& code);

        float m_Angle = 0.0;
        float m_Distance = 1.0;

        /// Options
        bool m_IsDuplicationOn = false;

        /// Debug option: count rendered frames
        size_t m_ElapsedFrames = 0;

        /// Debug option: terminate process after N frames (or go for infty in case of 0)
        size_t m_ExitAfterFrames = 0;

        ShaderManager m_Manager;
        FramebufferTracker m_FBOTracker;

        struct ViewportArea 
        {
            GLint data[4];

            ViewportArea()
            {
                for(size_t i = 0; i< 3;i++)
                    data[i] = 0;
            }

            GLint* getDataPtr() 
            {
                return data;
            }
            void set(GLint x, GLint y, GLsizei width, GLsizei height)
            {
                this->data[0] = x;
                this->data[1] = y;
                this->data[2] = width;
                this->data[3] = height;
            }
            
            GLint getX() const { return data[0]; }
            GLint getY() const { return data[1]; }
            GLint getWidth() const { return data[2]; }
            GLint getHeight() const { return data[3]; }
        }; 
        ViewportArea currentViewport, currentScissorArea;
        
    };
}
