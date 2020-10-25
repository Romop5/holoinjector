#include <functional>
#include "hooking/opengl_redirector_base.hpp"
#include <unordered_map>

#include <GL/gl.h>
#include <glm/glm.hpp>

#include "trackers/shader_manager.hpp"
#include "trackers/uniform_block_tracing.hpp"
#include "trackers/framebuffer_tracker.hpp"
#include "trackers/legacy_tracker.hpp"
#include "pipeline/viewport_area.hpp"
#include "pipeline/virtual_cameras.hpp"
#include "pipeline/output_fbo.hpp"

#include "diagnostics.hpp"

namespace ve
{
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

        virtual void glCompileShader (GLuint shader) override;

        // Map program to vertex shader
        virtual  void glAttachShader (GLuint program, GLuint shader) override;
        // Shader end 

        virtual  GLuint glCreateProgram (void) override;
        virtual  void glUseProgram (GLuint program) override;
        virtual void glLinkProgram (GLuint program) override;

        // Framebuffers
        virtual  void glGenFramebuffers (GLsizei n, GLuint* framebuffers) override;
        virtual  void glBindFramebuffer (GLenum target, GLuint framebuffer) override;
        virtual  void glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level) override;
        virtual  void glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) override;
        virtual  void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) override;
        virtual  void glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) override;
        // Framebuffers end

        
        // Binding tracing
        // Get Uniform Block location in shader program
        virtual GLuint glGetUniformBlockIndex (GLuint program, const GLchar* uniformBlockName) override;
        // Get binding slot for given block location
        virtual void glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) override;

        virtual void glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) override;
        virtual void glBindBufferBase (GLenum target, GLuint index, GLuint buffer) override;

        // Assign currently bind buffer to range of binding blocks
        virtual void glBindBuffersBase (GLenum target, GLuint first, GLsizei count, const GLuint* buffers) override;
        // Assign currently bind buffer to range of binding blocks
        virtual void glBindBuffersRange (GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes) override;

        virtual void glBufferData (GLenum target, GLsizeiptr size, const void* data, GLenum usage) override;
        virtual void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void* data) override;
        // Binding end

        // Draw calls start
        virtual void glDrawArrays(GLenum mode,GLint first,GLsizei count) override;
        virtual void glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) override;
        virtual  void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) override;
        virtual  void glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) override;

        virtual void glDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) override;
        virtual void glDrawRangeElementsBaseVertex (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex) override;
        virtual void glDrawElementsInstancedBaseVertex (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) override;
        virtual void glMultiDrawElementsBaseVertex (GLenum mode, const GLsizei* count, GLenum type, const void* const*indices, GLsizei drawcount, const GLint* basevertex) override;

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

        ///////////////////////////////////////////////////////////////////////
        // Internal routines
        ///////////////////////////////////////////////////////////////////////
        private:
        /// Initialize parameters, caches etc
        void initialize();

        void setEnhancerShift(const glm::mat4& viewSpaceTransform, float projectionAdjust = 0.0);
        void resetEnhancerShift();
        void setEnhancerIdentity();

        void setEnhancerDecodedProjection(GLuint program, const PerspectiveProjectionParameters& projection);

        void takeScreenshot(const std::string filename);

        void drawMultiviewed(const std::function<void(void)>& code);

        
        ///////////////////////////////////////////////////////////////////////
        // OPTIONS
        ///////////////////////////////////////////////////////////////////////

        /// Is repeater rendering scene into multiple virtual screens
        bool m_IsMultiviewActivated = false;
        
        /// Determines how virtual views are placed in view-space
        CameraParameters m_cameraParameters;

        /// Store's repeating setup
        VirtualCameras m_cameras;

        /// Provides interface for system testing
        Diagnostics m_diagnostics;

        ///////////////////////////////////////////////////////////////////////
        // OpenGL hooking structures
        ///////////////////////////////////////////////////////////////////////

        /// Store metadata about application's shaders and programs
        ShaderManager m_Manager;
        /// Store metadata about create Frame Buffer Objects
        FramebufferTracker m_FBOTracker;
        /// Keeps track of OpenGL fixed-pipeline calls
        LegacyTracker m_LegacyTracker;

        /// Store metadata and bindings for UBO
        UniformBlockTracing m_UniformBlocksTracker;

        /// Caches current viewport/scissor area
        ViewportArea currentViewport, currentScissorArea;
        /* 
         * Global glCallList for legacy OpenGL primitives
         * - record everything between glBegin()/glEnd() 
         *   and then, duplicate it
         */
        GLint m_callList = 0;

	/// FBO with all raw virtual cameras
	OutputFBO m_OutputFBO;
    };
}
