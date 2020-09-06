#include <functional>
#include "opengl_redirector_base.hpp"

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
        virtual void glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length);


        virtual void glDrawArrays(GLenum mode,GLint first,GLsizei count) override;
        virtual void glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) override;


        private:
        void duplicateCode(const std::function<void(void)>& code);
    };
}
