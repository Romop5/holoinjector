#include "opengl_redirector_base.hpp"

namespace ve
{
    class Repeater: public OpenglRedirectorBase
    {
        public:
        virtual void registerCallbacks(SymbolRedirection& redirector) override;

        virtual void glClear(GLbitfield mask) override;
        virtual void glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length);
    };
}
