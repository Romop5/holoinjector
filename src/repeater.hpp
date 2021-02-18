#include <functional>
#include <unordered_map>

#include <GL/gl.h>
#include <glm/glm.hpp>

#include "hooking/opengl_redirector_base.hpp"
#include "managers/draw_manager.hpp"
#include "managers/ui_manager.hpp"
#include "context.hpp"

namespace ve
{
    /**
     * @brief Reroutes hooked OpenGL calls to submodules
     *
     * Repeater serves as a dispatcher, redirecting OpenGL's API calls to
     * corresponding trackers and managers.
     *
     * Trackers keep track of object' properties and bindings whereas 
     * managers are responsible for handling events w.r.t. context.
     */
    class Repeater: public ve::hooking::OpenglRedirectorBase
    {
        public:
        virtual void registerCallbacks() override;

        ///////////////////////////////////////////////////////////////////////
        // Hooked-function handlers
        ///////////////////////////////////////////////////////////////////////
        // Used for initialization
        virtual void glClear(GLbitfield mask) override;

        // X11 library & GLX extension handlers
        virtual GLXContext glXCreateContext(Display * dpy, XVisualInfo * vis, GLXContext shareList, Bool direct) override;
        virtual void glXSwapBuffers(Display * dpy, GLXDrawable drawable) override;
        virtual Bool glXMakeCurrent(Display * dpy, GLXDrawable drawable,GLXContext ctx) override;
        virtual Bool glXMakeContextCurrent(Display * display, GLXDrawable draw, GLXDrawable read, GLXContext ctx) override;
        virtual int XNextEvent(Display *display, XEvent *event_return) override;
        virtual int XWarpPointer(Display* display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y) override;
        virtual Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int classInstance, Visual *visual, unsigned long valuemask, XSetWindowAttributes *        attributes) override;
        virtual void XSetWMNormalHints(Display *display, Window w, XSizeHints* hints);

        // Textures
        virtual void glGenTextures(GLsizei n,GLuint* textures) override;

        virtual void glTexImage1D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLint border,GLenum format,GLenum type,const GLvoid* pixels) override;
        virtual void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const GLvoid* pixels) override;
        virtual void glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) override;

        virtual void glTexStorage1D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) override;
        virtual void glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override;
        virtual void glTexStorage3D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) override;

        virtual void glTextureStorage1D (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width) override;
        virtual void glTextureStorage2D (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override;
        virtual void glTextureStorage3D (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) override;
 
        virtual void glBindTexture(GLenum target,GLuint texture) override;
        virtual void glActiveTexture (GLenum texture) override;

        // Renderbuffers
        virtual void glGenRenderbuffers (GLsizei n, GLuint* renderbuffers) override;
        virtual void glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) override;
        

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
        virtual void glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) override;
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
        virtual void glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instancecount) override;

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

        // Legacy fixed-pipeline OpenGL state tracking
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


        // Legacy OpenGL fixed-pipeline wihout VBO and VAO
        virtual void glBegin(GLenum mode) override;
        virtual void glEnd() override;
        // Legacy end

        ///////////////////////////////////////////////////////////////////////
        // Internal routines
        ///////////////////////////////////////////////////////////////////////
        private:
        GLint getCurrentID(GLenum target);
        /// Initialize parameters, caches etc
        void initialize();
        void deinitialize();

        void takeScreenshot(const std::string filename);
        void drawMultiviewed(const std::function<void(void)>& code);

        ///////////////////////////////////////////////////////////////////////
        // OpenGL structures
        ///////////////////////////////////////////////////////////////////////
        Context m_Context;

        ve::managers::DrawManager m_DrawManager;
        ve::managers::UIManager m_UIManager;

        struct {
            int m_LastXPosition = 0;
            int m_LastYPosition = 0;
        } X11MouseHook;
    };
} // namespace ve
