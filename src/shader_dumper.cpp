#include "shader_dumper.hpp"
namespace helper
{
    void dumpShaderSources(const GLchar* const* strings, GLsizei count)
    {
        for(size_t i=0; i < count; i++)
        {
            printf("Dumping shader source i: %d\n", i);
            puts(strings[i]);
            printf("---\n");
        }
    }
} // namespace helper


void ShaderDumper::registerCallbacks() 
{
    registerOpenGLSymbols();
}

GLuint ShaderDumper::glCreateShader(GLenum shaderType)
{
    auto id = OpenglRedirectorBase::glCreateShader(shaderType);

    ShaderMetadata metadata;
    metadata.isVertexShader = shaderType == GL_VERTEX_SHADER;
    m_shaderDatabase[id] = metadata;
    return id;
}

void ShaderDumper::glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length)
{
    printf("[ShaderDumper] glShaderSource: \n");
    helper::dumpShaderSources(string, count);
    OpenglRedirectorBase::glShaderSource(shader,count,string,length);
}


