#include "repeater.hpp"
#include <iostream>
#include <cassert>
#include <regex>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "shader_inspector.hpp"
#include "projection_estimator.hpp"
#include "opengl_utils.hpp"

using namespace ve;

namespace helper
{
    bool containsMainFunction(const std::string& shader)
    {
        // Search for vector assignment to gl_Position
        static std::regex glPositionRegex("gl_Position *=",
                std::regex_constants::ECMAScript | std::regex_constants::icase);
        return (std::regex_search(shader, glPositionRegex));
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

    float getEnviromentValue(const std::string& variable, float defaultValue = 0.0f)
    {
        auto envStringRaw = getenv(variable.c_str());
        if(!envStringRaw)
            return defaultValue;
        float resultValue;
        try {
            resultValue = std::stof(envStringRaw);
        } catch(...)
        {
            return defaultValue;
        }
        return resultValue;
    }

    template<typename T>
    void dumpOpenglMatrix(const T* m)
    {
        printf("[Repeater] Matrix: \n");
        printf("[Repeater] %f %f %f %f\n", m[0],m[4],m[8],m[12]);
        printf("[Repeater] %f %f %f %f\n", m[1],m[5],m[9],m[13]);
        printf("[Repeater] %f %f %f %f\n", m[2],m[6],m[10],m[14]);
        printf("[Repeater] %f %f %f %f\n", m[3],m[7],m[11],m[15]);
    }
} // namespace helper

void Repeater::registerCallbacks() 
{
    registerOpenGLSymbols();

    m_Angle = helper::getEnviromentValue("ENHANCER_ANGLE", m_Angle); 
    m_Distance = helper::getEnviromentValue("ENHANCER_DISTANCE", m_Distance); 
    m_ExitAfterFrames = helper::getEnviromentValue("ENHANCER_EXIT_AFTER", 0); 
}


void Repeater::glXSwapBuffers(	Display * dpy, GLXDrawable drawable)
{
    OpenglRedirectorBase::glXSwapBuffers(dpy, drawable);
    m_ElapsedFrames++; 
    if(m_ExitAfterFrames && m_ExitAfterFrames <= m_ElapsedFrames)
    {
        // Note: this is debug only, leaves mem. leaks and uncleaned objects
        exit(5);
    }
}

GLuint Repeater::glCreateShader(GLenum shaderType)
{
    auto id = OpenglRedirectorBase::glCreateShader(shaderType);
    
    printf("[Repeater] glCreateShader: %s\n",opengl_utils::getEnumStringRepresentation(shaderType).c_str());
    
    m_Manager.addShader(id, (shaderType == GL_VERTEX_SHADER)?(ShaderManager::ShaderTypes::VS):(ShaderManager::ShaderTypes::GENERIC));
    return id;
}

void Repeater::glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length)
{
    printf("[Repeater] glShaderSource: [%d]\n",shader);
    if(m_Manager.hasShader(shader) && m_Manager.getShaderDescription(shader).m_Type == ShaderManager::ShaderTypes::VS)
    {
        std::string newShader;
        std::vector<const GLchar*> preparedShaders;
        preparedShaders.reserve(count);

        std::vector<GLint> preparedShadersLengths;
        preparedShadersLengths.reserve(count);
        for(size_t cnt = 0; cnt < count; cnt++)
        {
            if(helper::containsMainFunction(string[cnt]))
            {
                newShader = string[cnt];

                printf("[Repeater] inspecting shader '%s'\n",newShader.c_str());
                ShaderInspector inspector(newShader);
                auto statements = inspector.findAllOutVertexAssignments();
                auto transformationName = inspector.getTransformationUniformName(statements);
                
                auto& metadata = m_Manager.getShaderDescription(shader);
                printf("[Repeater] found transformation name: %s\n",transformationName.c_str());
                metadata.m_TransformationMatrixName = transformationName;

                newShader = inspector.injectShader(statements);
                auto & description = m_Manager.getShaderDescription(shader);
                description.m_HasAnyUniform = (inspector.getCountOfUniforms() > 0);
                
                printf("[Repeater] injecting shader shift\n");
                preparedShaders.push_back(newShader.data());
                preparedShadersLengths.push_back(newShader.size());
            } else {
                preparedShaders.push_back(string[cnt]);
                auto shaderLength = (!length)?strlen(string[cnt]):length[cnt];
                preparedShadersLengths.push_back(shaderLength);
            }
        }
        helper::dumpShaderSources(preparedShaders.data(), count);
        OpenglRedirectorBase::glShaderSource(shader,count,preparedShaders.data(),preparedShadersLengths.data());
    } else {
        helper::dumpShaderSources(string, count);
        OpenglRedirectorBase::glShaderSource(shader,count,string,length);
    }
}

void Repeater::glAttachShader (GLuint program, GLuint shader)
{
    printf("[Repeater] attaching shader [%d] to program [%d] \n", shader, program);
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

    auto typeLocation= OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_isOrthogonal");
    OpenglRedirectorBase::glUniform1i(typeLocation,!estimatedParameters.isPerspective);
}


GLuint Repeater::glCreateProgram (void)
{
    auto result = OpenglRedirectorBase::glCreateProgram();
    m_Manager.addProgram(result);
    return result;
}

void Repeater::glUseProgram (GLuint program)
{
    OpenglRedirectorBase::glUseProgram(program);
    m_Manager.bind(program);
}


void Repeater::glViewport(GLint x,GLint y,GLsizei width,GLsizei height)
{
    OpenglRedirectorBase::glViewport(x,y,width,height);
    //currentViewport.set(x,y,width,height);
}

void Repeater::glScissor(GLint x,GLint y,GLsizei width,GLsizei height)
{
    OpenglRedirectorBase::glScissor(x,y,width,height);
    //currentScissorArea.set(x,y,width,height);
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

void Repeater::glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawRangeElements(mode,start,end,count,type,indices);
    });
}

void Repeater::glGenFramebuffers (GLsizei n, GLuint* framebuffers)
{
    OpenglRedirectorBase::glGenFramebuffers(n, framebuffers);
    for(size_t i=0;i < n; i++)
    {
        m_FBOTracker.addFramebuffer(framebuffers[i]);
    }
}

void Repeater::glBindFramebuffer (GLenum target, GLuint framebuffer)
{
    OpenglRedirectorBase::glBindFramebuffer(target,framebuffer);
    m_FBOTracker.bind(framebuffer);
}

void Repeater::glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture(target,attachment, texture,level);
    m_FBOTracker.attach(attachment, texture);
}

void Repeater::glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture1D(target,attachment,textarget, texture,level);
    m_FBOTracker.attach(attachment, texture);
}
void Repeater::glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture2D(target,attachment,textarget, texture,level);
    m_FBOTracker.attach(attachment, texture);
}
void Repeater::glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    OpenglRedirectorBase::glFramebufferTexture3D(target,attachment,textarget, texture,level,zoffset);
    m_FBOTracker.attach(attachment, texture);
}

void Repeater::glMatrixMode(GLenum mode)
{
    OpenglRedirectorBase::glMatrixMode(mode);
    m_LegacyTracker.matrixMode(mode);
    printf("[Repeater] glMatrixMode %s\n", ve::opengl_utils::getEnumStringRepresentation(mode).c_str());
}
void Repeater::glLoadMatrixd(const GLdouble* m)
{
    OpenglRedirectorBase::glLoadMatrixd(m);
    GLfloat newM[16];

    for(size_t i=0;i < 16;i++)
    {
        newM[i] = static_cast<float>(m[i]);
    }
    glm::mat4 result;
    std::memcpy(glm::value_ptr(result), newM, 16*sizeof(GLfloat));
    m_LegacyTracker.loadMatrix(std::move(result));

    //helper::dumpOpenglMatrix(m);
}
void Repeater::glLoadMatrixf(const GLfloat* m)
{
    OpenglRedirectorBase::glLoadMatrixf(m);
    glm::mat4 result;
    std::memcpy(glm::value_ptr(result), m, 16);
    m_LegacyTracker.loadMatrix(std::move(result));

    //helper::dumpOpenglMatrix(m);
}

//-----------------------------------------------------------------------------
// Debugging utils
//-----------------------------------------------------------------------------
int Repeater::XNextEvent(Display *display, XEvent *event_return)
{
    auto returnVal = OpenglRedirectorBase::XNextEvent(display,event_return);

    if(event_return->type == KeyPress)
    {
        auto keyEvent = reinterpret_cast<XKeyEvent*>(event_return);
        static unsigned long lastSerial = 0;
        if(keyEvent->serial == lastSerial)
        {
            return returnVal;
        } else {
            lastSerial = keyEvent->serial;
        }
        printf("[Repeater] XNextEvent KeyPress %d, %lu - %d - %p - [%d %d] [%u %u] %d\n",
                keyEvent->type,keyEvent->serial, keyEvent->send_event,keyEvent->display, 
                keyEvent->x,keyEvent->y,keyEvent->state,keyEvent->keycode, keyEvent->same_screen);
        auto keySym = XLookupKeysym(reinterpret_cast<XKeyEvent*>(event_return), 0);
        switch(keySym)
        {
            case XK_F1:
            {
                m_Angle += 0.01;
                puts("[Repeater] Setting: F1 pressed - increase angle");
            }
            break;
            case XK_F2:
            {
                m_Angle -= 0.01;
                puts("[Repeater] Setting: F2 pressed - decrease angle");
            }
            break;

            case XK_F3:
            {
                m_Distance += 0.1;
                puts("[Repeater] Setting: F3 pressed increase dist");
            }
            break;

            case XK_F4:
            {
                m_Distance -= 0.1;
                puts("[Repeater] Setting: F4 pressed decrease dist");
            }
            break;

            case XK_F5:
            {
                m_Distance = 1.0;
                m_Angle = 0.0;
                puts("[Repeater] Setting: F5 pressed - reset");
                break;
            }

            case XK_F12:
            case XK_F11:
            {
                m_IsDuplicationOn = !m_IsDuplicationOn;
                puts("[Repeater] Setting: F11 pressed - toggle");
            }
            break;
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

void Repeater::setEnhancerShift(float rotationAroundX, float rotationAroundY)
{
    const auto rotX = glm::rotate(rotationAroundX, glm::vec3(1,0,0));
    const auto rotY = glm::rotate(rotationAroundY, glm::vec3(0,1,0));
    const auto cameraRotation = rotY*rotX;
    float dist = m_Distance;
    auto T = glm::translate(glm::vec3(0.0f,0.0f,dist));
    auto invT = glm::translate(glm::vec3(0.0f,0.0f,-dist));

    //auto resultMat = clipSpaceTransformation;
    auto resultMat = invT*cameraRotation*T;
    auto program = getCurrentProgram();
    auto location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_view_transform");
    OpenglRedirectorBase::glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(glm::value_ptr(resultMat)));

    location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_identity");
    OpenglRedirectorBase::glUniform1i(location, GL_FALSE);

    // Legacy support
    /*
     * multiply GL_PROJECTION from right
     * Note: GL_PROJECTION_STACK_DEPTH must be at least 2
     *
     * Note: sometimes GL_PROJECTION may not be well-shaped
     * e.g. when porting DirectX game to OpenGL
     */

    if(m_LegacyTracker.isLegacyNeeded())
    {
        OpenglRedirectorBase::glMatrixMode(GL_PROJECTION);
        OpenglRedirectorBase::glPushMatrix();
        if(!m_LegacyTracker.isOrthogonalProjection())
            OpenglRedirectorBase::glMultMatrixf(glm::value_ptr(resultMat));
    }
}


void Repeater::resetEnhancerShift()
{
    if(m_LegacyTracker.isLegacyNeeded())
        OpenglRedirectorBase::glPopMatrix();
}

void Repeater::setEnhancerIdentity()
{
    const auto identity = glm::mat4(1.0);
    auto program = getCurrentProgram();
    auto location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_identity");
    OpenglRedirectorBase::glUniform1i(location, GL_TRUE);
}


void Repeater::duplicateCode(const std::function<void(void)>& code)
{
    bool shouldNotDuplicate = (
            !m_IsDuplicationOn ||
            // don't duplicate while rendering light's point of view into shadow map
            m_FBOTracker.isFBOshadowMap() ||
            // don't duplicate while rendering post-processing effect
            (m_Manager.isAnyBound() && m_Manager.isVSBound() && !m_Manager.getBoundedVS().m_HasAnyUniform));

    if(shouldNotDuplicate)
    {
        setEnhancerIdentity();
        code();
        return;
    }

    // Get original viewport
    glGetIntegerv(GL_VIEWPORT, currentViewport.getDataPtr());
    glGetIntegerv(GL_SCISSOR_BOX, currentScissorArea.getDataPtr());
    auto originalViewport = currentViewport;
    auto originalScissor = currentScissorArea;

    /*
     * Define viewports
     */

    constexpr size_t tilesPerX = 3;

    struct Views
    {
        float angleX;
        float angleY;
    };

   /*
   std::array<Views,2> views 
    { {
        {0.0f,-1.0f},
        {0.0f,1.0f}
      } };
    
     
    std::array<Views,3> views 
    { {
        {0.0f,-1.0f},
        {0.0f,0.0f},
        {0.0f,1.0f}
      } };
    
    */ 
    /*
    // XY axis
    std::array<Views,9> views 
    { {



        {-1.0f,1.0f},
        {-1.0f,0.0f},
        {-1.0f,-1.0f},

        {0.0f,1.0f},
        {0.0f,0.0f},
        {0.0f,-1.0f},

        {1.0f,1.0f},
        {1.0f,0.0f},
        {1.0f,-1.0f},

      } };
      */

    // Only X-axis
    
    std::array<Views,9> views 
    { {

        {0.0f,4.0f},
        {0.0f,3.0f},
        {0.0f,2.0f},

        {0.0f,1.0f},
        {0.0f,0.0f},
        {0.0f,-1.0f},

        {0.0f,-2.0f},
        {0.0f,-3.0f},
        {0.0f,-1.0f},

      } };
      
    

    

    constexpr size_t tilesPerY = views.size()/tilesPerX + ((views.size() % tilesPerX) > 0);

    const size_t width = originalViewport.getWidth()/tilesPerX;
    const size_t height= originalViewport.getHeight()/tilesPerY;

    const size_t startX= originalViewport.getX();
    const size_t startY= originalViewport.getY();

    for(size_t i = 0; i < views.size(); i++)
    {
        const Views& currentView = views[i];
        size_t posX = i % tilesPerX;
        size_t posY = i / tilesPerX;

        const size_t currentStartX = startX + posX*width;
        const size_t currentStartY = startY + posY*height;
        OpenglRedirectorBase::glViewport(currentStartX, currentStartY, width, height);
        const auto scissorDiffX = (originalScissor.getX()-originalViewport.getX())/tilesPerX;
        const auto scissorDiffY = (originalScissor.getY()-originalViewport.getY())/tilesPerY;
        const auto scissorWidth = originalScissor.getWidth()/tilesPerX; 
        const auto scissorHeight = originalScissor.getHeight()/tilesPerY; 
        OpenglRedirectorBase::glScissor(currentStartX+scissorDiffX, currentStartY+scissorDiffY, scissorWidth, scissorHeight);

        const auto rotXAngle = m_Angle*currentView.angleX;
        const auto rotYAngle = m_Angle*currentView.angleY;
        setEnhancerShift(rotXAngle, rotYAngle);
        code();
        resetEnhancerShift();
    }
    // restore
    OpenglRedirectorBase::glViewport(originalViewport.getX(), originalViewport.getY(), originalViewport.getWidth(), originalViewport.getHeight());
    OpenglRedirectorBase::glScissor(originalScissor.getX(), originalViewport.getY(), originalScissor.getWidth(), originalScissor.getHeight());
}

