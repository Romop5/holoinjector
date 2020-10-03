#include <functional>
#include "opengl_redirector_base.hpp"
#include <unordered_map>

#include <GL/gl.h>
#include <glm/glm.hpp>

#include "shader_manager.hpp"
#include "framebuffer_tracker.hpp"
#include "legacy_tracker.hpp"
#include "viewport_area.hpp"
#include "virtual_cameras.hpp"

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

        // Used for initialization
        virtual  void glClear(GLbitfield mask) override;

        // Swap buffers
        virtual  void glXSwapBuffers(	Display * dpy, GLXDrawable drawable) override;

        // Shader start
        virtual  GLuint glCreateShader(GLenum shaderType) override;
        virtual  void glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length) override;

        // Map program to vertex shader
        virtual  void glAttachShader (GLuint program, GLuint shader) override;
        // Shader end 

        virtual  GLuint glCreateProgram (void) override;
        virtual  void glUseProgram (GLuint program) override;

        // Framebuffers
        virtual  void glGenFramebuffers (GLsizei n, GLuint* framebuffers) override;
        virtual  void glBindFramebuffer (GLenum target, GLuint framebuffer) override;
        virtual  void glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level) override;
        virtual  void glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) override;
        virtual  void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) override;
        virtual  void glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) override;
        // Framebuffers end

        // Draw calls start
        virtual void glDrawArrays(GLenum mode,GLint first,GLsizei count) override;
        virtual void glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) override;
        virtual  void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) override;
        virtual  void glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) override;

        virtual  void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;


        // Viewport start
        virtual  void glViewport(GLint x,GLint y,GLsizei width,GLsizei height) override;
        virtual  void glScissor(GLint x,GLint y,GLsizei width,GLsizei height) override;
        // Viewport end 
        virtual  void glMatrixMode(GLenum mode) override;
        virtual  void glLoadMatrixd(const GLdouble* m) override;
        virtual  void glLoadMatrixf(const GLfloat* m) override;
        virtual void glLoadIdentity(void) override;
        virtual void glMultMatrixd(const GLdouble* m) override;
        virtual void glMultMatrixf(const GLfloat* m) override;

        virtual void glOrtho(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val) override;
        virtual void glFrustum(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val) override;
        virtual void glCallList(GLuint list) override;
        virtual void glCallLists(GLsizei n,GLenum type,const GLvoid* lists) override;
        
        virtual  int XNextEvent(Display *display, XEvent *event_return) override;

        // Legacy OpenGL fixed-pipeline wihout VBO and VAO
        virtual void glBegin(GLenum mode) override;
        virtual void glEnd() override;
        // Legacy end

        private:
        /// Build cache structures
        void initialize();

        GLint getCurrentProgram();
        void setEnhancerShift(const glm::mat4& viewSpaceTransform);
        void resetEnhancerShift();
        void setEnhancerIdentity();

        void duplicateCode(const std::function<void(void)>& code);

        /// Options
        bool m_IsDuplicationOn = false;

        /// Debug option: count rendered frames
        size_t m_ElapsedFrames = 0;

        /// Debug option: terminate process after N frames (or go for infty in case of 0)
        size_t m_ExitAfterFrames = 0;

        ShaderManager m_Manager;
        FramebufferTracker m_FBOTracker;
        /// Keeps track of OpenGL fixed-pipeline calls
        LegacyTracker m_LegacyTracker;

        CameraParameters m_cameraParameters;
        /// Store's repeating setup
        VirtualCameras m_cameras;

        ViewportArea currentViewport, currentScissorArea;
        /// Is scissor region different 
        bool m_isScissorRegionActive = false;
        
        /* 
         * Global glCallList for legacy OpenGL primitives
         * - record everything between glBegin()/glEnd() 
         *   and then, duplicate it
         */
        GLint m_callList = 0;
    };
}
