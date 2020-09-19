#include "repeater.hpp"
#include <iostream>
#include <cassert>
#include <regex>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "shader_inspector.hpp"
#include "projection_estimator.hpp"

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
        //shader.insert(mainPosition, "uniform vec3 enhancer_shift;\n");
        //mainPosition = shader.find("void main()");

        shader.insert(mainPosition, "uniform mat4 enhancer_transform;\n");
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
            shader.insert(braceIterator-shader.begin(), "\n\tvec3 enhancer_shift = vec3(0.0,0.0,0.1)*gl_Position.w; \n\tgl_Position.xyz = (enhancer_transform*(vec4(enhancer_shift+gl_Position.xyz,1.0))).xyz-enhancer_shift;\n");
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
    m_Manager.addShader(id, (shaderType == GL_VERTEX_SHADER)?(ShaderManager::ShaderTypes::VS):(ShaderManager::ShaderTypes::GENERIC));
    return id;
}

void Repeater::glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length)
{
    printf("[Repeater] glShaderSource: \n");
    if(m_Manager.hasShader(shader) && m_Manager.getShaderDescription(shader).m_Type == ShaderManager::ShaderTypes::VS)
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
                //helper::insertEnhancerShift(newShader);
                newShader = string[cnt];

                printf("[Repeater] inspecting shader '%s'\n",newShader.c_str());
                ShaderInspector inspector(newShader);
                auto statements = inspector.findAllOutVertexAssignments();
                auto transformationName = inspector.getTransformationUniformName(statements);
                
                auto& metadata = m_Manager.getShaderDescription(shader);
                printf("[Repeater] found transformation name: %s\n",transformationName.c_str());
                metadata.m_TransformationMatrixName = transformationName;

                newShader = inspector.injectShader(statements);
                
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

void Repeater::glAttachShader (GLuint program, GLuint shader)
{
    OpenglRedirectorBase::glAttachShader(program,shader);

    if(!m_Manager.hasProgram(program))
        return;
    if(!m_Manager.hasShader(shader))
        return;
    if(m_Manager.getShaderDescription(shader).m_Type != ShaderManager::ShaderTypes::VS)
        return;
    m_Manager.attachShaderToProgram(shader, program);
}


void Repeater::glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    printf("[Repeater] generic uniform matrix\n");
    OpenglRedirectorBase::glUniformMatrix4fv (location, count, transpose, value);

    // get current's program transformation matrix name
    auto program = getCurrentProgram();
    if(!m_Manager.hasProgram(program))
        return;
    const auto vertexShaderID = m_Manager.getProgram(program).m_VertexShader;
    if(vertexShaderID == -1 || !m_Manager.hasShader(vertexShaderID))
        return;
    auto shaderMetaData = m_Manager.getShaderDescription(vertexShaderID);
    if(shaderMetaData.m_TransformationMatrixName == "")
        return;

    // get original MVP matrix location
    auto originalLocation = OpenglRedirectorBase::glGetUniformLocation(program, shaderMetaData.m_TransformationMatrixName.c_str());
    
    // if the matrix being uploaded isn't detected MVP, then continue
    if(originalLocation != location)
        return;

    // estimate projection matrix from value
    glm::mat4 mat;
    std::memcpy(glm::value_ptr(mat), value, 16*sizeof(float));
    auto estimatedParameters = estimatePerspectiveProjection(mat);

    printf("[Repeater] estimating parameters from uniform matrix\n");
    printf("[Repeater] parameters: fx(%f) fy(%f) near (%f) near(%f) \n", estimatedParameters.fx, estimatedParameters.fy, estimatedParameters.nearPlane, estimatedParameters.farPlane);

    // upload parameters to GPU's program
    auto parametersLocation = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_estimatedParameters");
    OpenglRedirectorBase::glUniform4fv(parametersLocation,1,glm::value_ptr(estimatedParameters.asVector()));
}


GLuint Repeater::glCreateProgram (void)
{
    auto result = OpenglRedirectorBase::glCreateProgram();
    m_Manager.addProgram(result);
    return result;
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


void Repeater::glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawElementsInstanced(mode,count,type,indices,instancecount);
    });
}

//-----------------------------------------------------------------------------
// Debugging utils
//-----------------------------------------------------------------------------
int Repeater::XNextEvent(Display *display, XEvent *event_return)
{
    auto returnVal = OpenglRedirectorBase::XNextEvent(display,event_return);
    if(event_return->type == KeyPress)
    {
        auto keySym = XLookupKeysym(reinterpret_cast<XKeyEvent*>(event_return), 0);
        if(keySym == XK_F1)
        {
            m_Angle += 0.01;
            puts("[Repeater] Setting: F1 pressed - increase angle");
        }
        if(keySym == XK_F2)
        {
            m_Angle -= 0.01;
            puts("[Repeater] Setting: F2 pressed - decrease angle");
        }

        if(keySym == XK_F3)
        {
            m_Distance += 0.1;
            puts("[Repeater] Setting: F3 pressed increase dist");
        }

        if(keySym == XK_F4)
        {
            m_Distance -= 0.1;
            puts("[Repeater] Setting: F4 pressed decrease dist");
        }

        if(keySym == XK_F5)
        {
            m_Distance = 1.0;
            m_Angle = 0.0;
            puts("[Repeater] Setting: F5 pressed - reset");
        }

        printf("[Repeater] Setting: dist (%f), angle (%f)\n", m_Distance, m_Angle);

    }
    return returnVal;
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

void Repeater::setEnhancerShift(const glm::vec3& clipSpaceTransformation)
{
    return;
    auto program = getCurrentProgram();
    auto location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_view_transform");

    glm::mat4 tmpMat = glm::mat4(glm::vec4(1,0,0,0),glm::vec4(0,1,0,0),glm::vec4(0,0,1,0),glm::vec4(clipSpaceTransformation,0.0));
    OpenglRedirectorBase::glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(glm::value_ptr(tmpMat)));
}
void Repeater::setEnhancerShift(const glm::mat4& clipSpaceTransformation)
{
    float dist = m_Distance;
    auto T = glm::translate(glm::vec3(0.0f,0.0f,dist));
    auto invT = glm::translate(glm::vec3(0.0f,0.0f,-dist));
    auto resultMat = invT*clipSpaceTransformation*T;
    auto program = getCurrentProgram();
    auto location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_view_transform");
    OpenglRedirectorBase::glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(glm::value_ptr(resultMat)));
}


void Repeater::duplicateCode(const std::function<void(void)>& code)
{
    
    GLint viewport[4];
    OpenglRedirectorBase::glGetIntegerv(GL_VIEWPORT, viewport);
    // Set bottom (y is reverted in OpenGL) 
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1], viewport[2],viewport[3]/2);
    //setEnhancerShift(glm::vec3(0.3,0,0));

    const glm::mat4 plusRotation = glm::rotate(-m_Angle, glm::vec3(0.f,1.0f,0.f));
    setEnhancerShift(plusRotation);
    code();
    // Set top
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1]+viewport[3]/2, viewport[2],viewport[3]/2);
    //setEnhancerShift(glm::vec3(-0.3,0,0));
    const glm::mat4 minusRotation = glm::rotate(+m_Angle,glm::vec3(0.f,1.0f,0.f));
    setEnhancerShift(minusRotation);
    code();
    // restore viewport
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1], viewport[2],viewport[3]);
}

