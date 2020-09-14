#include "opengl_redirector_base.hpp"

namespace ve
{
    class ShaderDumper: public OpenglRedirectorBase
    {
        public:
        virtual void registerCallbacks() override;

        virtual GLuint glCreateShader(GLenum shaderType);
        virtual void glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length);
    };
}
