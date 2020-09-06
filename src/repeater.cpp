#include "repeater.hpp"
#include <iostream>
#include <cassert>

#include <regex>

using namespace ve;

void Repeater::registerCallbacks() 
{
    registerOpenGLSymbols();
}

void Repeater::glClear(GLbitfield mask)
{
    printf("[Repeater] Redirecting glClear\n");
    OpenglRedirectorBase::glClear(mask);
} 

namespace helper
{
    bool containsMainFunction(const std::string& shader)
    {
        // Search for vector assignment to gl_Position
        static std::regex glPositionRegex("gl_Position *=",
                std::regex_constants::ECMAScript | std::regex_constants::icase);
        return (std::regex_search(shader, glPositionRegex));
    }

    void insertEnhancerShift(std::string& shader)
    {
        auto mainPosition = shader.find("void main()");
        if(mainPosition == std::string::npos)
            return;
        shader.insert(mainPosition, "uniform vec3 enhancer_shift;\n");
        mainPosition = shader.find("void main()");

        auto firstBrace = shader.find("{", mainPosition);
        auto braceIterator = shader.begin()+firstBrace+1;
        size_t braceCount = 1;
        while(braceIterator != shader.end() && braceCount > 0)
        {
            if(*braceIterator == '}')
                braceCount--;

            if(*braceIterator == '{')
                braceCount++;

            braceIterator++;
        }

        if(braceCount == 0)
        {
            braceIterator--;
            shader.insert(braceIterator-shader.begin(), "\n\tgl_Position.xyz += enhancer_shift;\n");
        }
    }

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

GLuint Repeater::glCreateShader(GLenum shaderType)
{
    auto id = OpenglRedirectorBase::glCreateShader(shaderType);

    ShaderMetadata metadata;
    metadata.isVertexShader = shaderType == GL_VERTEX_SHADER;
    m_shaderDatabase[id] = metadata;
    return id;
}

void Repeater::glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length)
{
    printf("[Repeater] glShaderSource: \n");
    if(m_shaderDatabase.count(shader) > 0 && m_shaderDatabase[shader].isVertexShader)
    {
        // when length is an array, handling is different 
        assert(length == nullptr);

        std::string newShader;
        std::vector<const GLchar*> preparedShaders;
        preparedShaders.reserve(count);
        for(size_t cnt = 0; cnt < count; cnt++)
        {
            if(helper::containsMainFunction(string[cnt]))
            {
                newShader = string[cnt];
                helper::insertEnhancerShift(newShader);
                printf("[Repeater] injecting shader shift\n");
                preparedShaders.push_back(newShader.data());
            } else {
                preparedShaders.push_back(string[cnt]);
            }
        }
        helper::dumpShaderSources(preparedShaders.data(), count);
        OpenglRedirectorBase::glShaderSource(shader,count,preparedShaders.data(),length);
    } else {
        helper::dumpShaderSources(string, count);
        OpenglRedirectorBase::glShaderSource(shader,count,string,length);
    }
}

//-----------------------------------------------------------------------------
// Duplicate API calls
//-----------------------------------------------------------------------------

void Repeater::glDrawArrays(GLenum mode,GLint first,GLsizei count) 
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawArrays(mode,first,count);
    });
}

void Repeater::glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) 
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawElements(mode,count,type,indices);
    });

}

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

GLint Repeater::getCurrentProgram()
{
    GLint id;
    OpenglRedirectorBase::glGetIntegerv(GL_CURRENT_PROGRAM,&id);
    return id;
}

void Repeater::setEnhancerShift(GLfloat x, GLfloat y, GLfloat z)
{
    auto program = getCurrentProgram();
    auto location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_shift");
    OpenglRedirectorBase::glUniform3f(location, x,y,z);
}


void Repeater::duplicateCode(const std::function<void(void)>& code)
{
    
    GLint viewport[4];
    OpenglRedirectorBase::glGetIntegerv(GL_VIEWPORT, viewport);
    // Set bottom (y is reverted in OpenGL) 
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1], viewport[2],viewport[3]/2);
    setEnhancerShift(0.3,0,0);
    code();
    // Set top
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1]+viewport[3]/2, viewport[2],viewport[3]/2);
    setEnhancerShift(-0.3,0,0);
    code();
    // restore viewport
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1], viewport[2],viewport[3]);
}

