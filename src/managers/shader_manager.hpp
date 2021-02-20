#ifndef VE_SHADER_MANAGER_HPP
#define VE_SHADER_MANAGER_HPP

#include <functional>

namespace ve
{
    class Context;

namespace managers
{
    class ShaderManager
    {
    public:
        GLuint createShader(Context& context, GLenum shaderType);
        void shaderSource (Context& context, GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length);
        void compileShader (Context& context, GLuint shader);

        // Map program to vertex shader
        void attachShader (Context& context, GLuint program, GLuint shader);
        // Shader end 

        GLuint createProgram (Context& context);
        void useProgram (Context& context, GLuint program);
        void linkProgram (Context& context, GLuint program);

    };
} // namespace managers
} // namespace ve

#endif
