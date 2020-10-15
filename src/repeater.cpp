#include "repeater.hpp"
#include <iostream>
#include <cassert>
#include <regex>
#include <unordered_set>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "FreeImage.h" // FreeImage image storing
#include "simplecpp.h" // CPP preprocessor

#include "shader_inspector.hpp"
#include "projection_estimator.hpp"
#include "opengl_utils.hpp"



using namespace ve;

namespace helper
{
    float getEnviromentValue(const std::string& variable, float defaultValue = 0.0f)
    {
        auto envStringRaw = getenv(variable.c_str());
        if(!envStringRaw)
            return defaultValue;
        float resultValue = defaultValue;
        try {
            resultValue = std::stof(envStringRaw);
        } catch(...) {};
        printf("[Enhancer] Getting env value of %s => %f\n", variable.c_str(),resultValue);
        return resultValue;
    }

    std::string getEnviromentValueStr(const std::string& variable, std::string defaultValue = "")
    {
        auto envStringRaw = getenv(variable.c_str());
        auto result = (envStringRaw)?envStringRaw:defaultValue;
        printf("[Enhancer] Getting env value of %s => %s\n", variable.c_str(),result.c_str());
        return result;
    }


    void getEnviroment(const std::string& variable, float& storage)
    {
        storage = getEnviromentValue(variable, storage);
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

    glm::mat4 createMatrixFromRawGL(const GLfloat* values)
    {
        glm::mat4 result;
        std::memcpy(glm::value_ptr(result), values, 16*sizeof(GLfloat));
        return result;
    }

    glm::mat4 createMatrixFromRawGL(const GLdouble* value)
    {
        GLfloat newM[16];
        for(size_t i=0;i < 16;i++)
        {
            newM[i] = static_cast<float>(value[i]);
        }
        glm::mat4 result;
        std::memcpy(glm::value_ptr(result), newM, 16*sizeof(GLfloat));
        return result;
    }

} // namespace helper

void Repeater::initialize()
{
    // Override default angle
    helper::getEnviroment("ENHANCER_XMULTIPLIER", m_cameraParameters.m_XShiftMultiplier); 
    // Override default center of rotation
    helper::getEnviroment("ENHANCER_DISTANCE", m_cameraParameters.m_frontOpticalAxisCentreDistance); 
    // if ENHANCER_NOW is provided, then start with multiple views right now
    if(helper::getEnviromentValue("ENHANCER_NOW", 0))
    {
        m_IsMultiviewActivated = true;
    }
    m_diagnostics.setTerminationAfterFrame(helper::getEnviromentValue("ENHANCER_EXIT_AFTER", 0)); 

    auto onlyCamera = helper::getEnviromentValue("ENHANCER_CAMERAID", -1);
    if(onlyCamera != -1)
    {
        m_diagnostics.setOnlyVirtualCamera(onlyCamera);
    }

    auto screenshotFormat = helper::getEnviromentValueStr("ENHANCER_SCREENSHOT");
    if(!screenshotFormat.empty())
    {
        m_diagnostics.setScreenshotFormat(screenshotFormat);
    }

    auto shouldNotBeIntrusive= helper::getEnviromentValue("ENHANCER_NONINTRUSIVE");
    if(shouldNotBeIntrusive)
    {
        m_diagnostics.setNonIntrusiveness(shouldNotBeIntrusive);
    }


    /// Fill viewports
    glGetIntegerv(GL_VIEWPORT, currentViewport.getDataPtr());
    glGetIntegerv(GL_SCISSOR_BOX, currentScissorArea.getDataPtr());
    m_cameras.setupWindows(9,3);
    m_cameras.updateViewports(currentViewport);
    m_cameras.updateParamaters(m_cameraParameters);
}

void Repeater::glClear(GLbitfield mask) 
{
    static bool isInitialized = false;
    if(!isInitialized)
    {
        initialize();
        isInitialized = true;
    }
    OpenglRedirectorBase::glClear(mask);
    
}
void Repeater::registerCallbacks() 
{
    registerOpenGLSymbols();
}

void Repeater::glXSwapBuffers(	Display * dpy, GLXDrawable drawable)
{
    OpenglRedirectorBase::glXSwapBuffers(dpy, drawable);
    m_diagnostics.incrementFrameCount(); 
    if(m_diagnostics.hasReachedLastFrame())
    {
        // Note: this is debug only, leaves mem. leaks and uncleaned objects
        takeScreenshot(std::string(m_diagnostics.getScreenshotName()));
        exit(5);
    }
}

GLuint Repeater::glCreateShader(GLenum shaderType)
{
    auto id = OpenglRedirectorBase::glCreateShader(shaderType);
    
    //printf("[Repeater] glCreateShader: %s\n",opengl_utils::getEnumStringRepresentation(shaderType).c_str());
    
    m_Manager.addShader(id, (shaderType == GL_VERTEX_SHADER)?(ShaderManager::ShaderTypes::VS):(ShaderManager::ShaderTypes::GENERIC));
    return id;
}

namespace helper
{
    std::string wrapGLSLMacros(std::string code)
    {
        auto regV = std::regex("#version");
        auto result = std::regex_replace (code,regV,"GLSL_VERSION",std::regex_constants::match_default);

        auto regE = std::regex("#extension");
        result = std::regex_replace (result,regE,"GLSL_EXTENSION",std::regex_constants::match_default);
        return result;
    }
    std::string unwrapGLSLMacros(std::string code)
    {
        auto regV = std::regex("GLSL_VERSION");
        auto result = std::regex_replace (code,regV,"#version",std::regex_constants::match_default);

        auto regE = std::regex("GLSL_EXTENSION");
        result = std::regex_replace (result,regE,"#extension",std::regex_constants::match_default);
        return result;
    }
    std::string preprocessGLSLCode(std::string code)
    {
        std::stringstream ss;
        ss << helper::wrapGLSLMacros(code);
        auto tmp = helper::unwrapGLSLMacros(simplecpp::preprocess_inmemory(ss));
        return std::regex_replace(tmp, std::regex("^$"),"");
    }
    
    std::string joinGLSLshaders(GLsizei count, const GLchar* const*string, const GLint* length)
    {
        std::stringstream ss;
        for(size_t i = 0; i < count; i++)
        {
            std::string_view file = length?std::string_view(string[i], length[i]):string[i];
            ss << file << "\n";
        }
        return ss.str();
    }
};

void Repeater::glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length)
{
    auto concatenatedShader = helper::joinGLSLshaders(count, string, length);
    printf("[Repeater] glShaderSource: [%d]\n",shader);
    if(m_Manager.hasShader(shader) && m_Manager.isShaderOneOf(shader, {ShaderManager::ShaderTypes::VS, ShaderManager::ShaderTypes::GEOMETRY}))
    {
        auto preprocessedShader = helper::preprocessGLSLCode(concatenatedShader);
        printf("[Repeater] preprocessed shader (type: %d) '%s'\n",m_Manager.getShaderDescription(shader).m_Type, preprocessedShader.c_str());

        ShaderInspector inspector(preprocessedShader);
        auto statements = inspector.findAllOutVertexAssignments();
        auto transformationName = inspector.getTransformationUniformName(statements);
        
        auto& metadata = m_Manager.getShaderDescription(shader);
        metadata.m_TransformationMatrixName = transformationName;
        metadata.m_IsClipSpaceTransform = inspector.isClipSpaceShader(); 
        metadata.m_InterfaceBlockName = inspector.getUniformBlockName(metadata.m_TransformationMatrixName);
        metadata.m_HasAnyUniform = (inspector.getCountOfUniforms() > 0);

        auto finalShader = preprocessedShader; 
        if(!m_diagnostics.shouldNotBeIntrusive())
            finalShader = inspector.injectShader(statements);
        printf("[Repeater] found transformation name: %s\n",metadata.m_TransformationMatrixName.c_str());
        printf("[Repeater] found interface block: %s\n",metadata.m_InterfaceBlockName.c_str());
        printf("[Repeater] injected shader: %s\n",finalShader.c_str());
        concatenatedShader = std::move(finalShader);
    }
    std::vector<const char*> shaders = {concatenatedShader.c_str(),};
    OpenglRedirectorBase::glShaderSource(shader,1,shaders.data(),nullptr);
}

void Repeater::glCompileShader (GLuint shader)
{
    printf("[Repeater] glCompileShader\n");
    OpenglRedirectorBase::glCompileShader(shader);
    GLint status;
    OpenglRedirectorBase::glGetShaderiv(shader, GL_COMPILE_STATUS,&status);
    if(status == GL_FALSE)
    {
        printf("[Repeater] Error while comping shader [%d]\n", shader);
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
    m_Manager.attachShaderToProgram(m_Manager.getShaderDescription(shader).m_Type,shader, program);
}


void Repeater::glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    OpenglRedirectorBase::glUniformMatrix4fv (location, count, transpose, value);

    // get current's program transformation matrix name
    if(!m_Manager.isVSBound())
        return;
    auto& shaderMetaData = m_Manager.getBoundedVS();
    if(!shaderMetaData.hasDetectedTransformation())
        return;

    auto programID = m_Manager.getBoundedProgramID();
    // get original MVP matrix location
    auto originalLocation = OpenglRedirectorBase::glGetUniformLocation(programID, shaderMetaData.m_TransformationMatrixName.c_str());
    
    // if the matrix being uploaded isn't detected MVP, then continue
    if(originalLocation != location)
        return;

    // estimate projection matrix from value
    const auto mat = helper::createMatrixFromRawGL(value);
    auto estimatedParameters = estimatePerspectiveProjection(mat);

    printf("[Repeater] estimating parameters from uniform matrix\n");
    printf("[Repeater] parameters: fx(%f) fy(%f) near (%f) near(%f) isPerspective (%d) \n", estimatedParameters.fx, estimatedParameters.fy, estimatedParameters.nearPlane, estimatedParameters.farPlane, estimatedParameters.isPerspective);

    setEnhancerDecodedProjection(programID, estimatedParameters);
}

void Repeater::setEnhancerDecodedProjection(GLuint program, const PerspectiveProjectionParameters& projection)
{
    // upload parameters to GPU's program
    auto parametersLocation = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_estimatedParameters");
    OpenglRedirectorBase::glUniform4fv(parametersLocation,1,glm::value_ptr(projection.asVector()));

    auto typeLocation= OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_isOrthogonal");
    OpenglRedirectorBase::glUniform1i(typeLocation,!projection.isPerspective);

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
    currentViewport.set(x,y,width,height);
    m_cameras.updateViewports(currentViewport);
}

void Repeater::glScissor(GLint x,GLint y,GLsizei width,GLsizei height)
{
    OpenglRedirectorBase::glScissor(x,y,width,height);
    currentScissorArea.set(x,y,width,height);
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

void Repeater::glDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawElementsBaseVertex(mode,count,type,indices, basevertex);
    });
}
void Repeater::glDrawRangeElementsBaseVertex (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawRangeElementsBaseVertex(mode,start,end,count,type,indices,basevertex);
    });
}
void Repeater::glDrawElementsInstancedBaseVertex (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) 
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawElementsInstancedBaseVertex(mode,count,type,indices, instancecount, basevertex);
    });
}

void Repeater::glMultiDrawElementsBaseVertex (GLenum mode, const GLsizei* count, GLenum type, const void* const*indices, GLsizei drawcount, const GLint* basevertex)
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glMultiDrawElementsBaseVertex(mode,count,type,indices, drawcount, basevertex);
    });
}

//
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
GLuint Repeater::glGetUniformBlockIndex (GLuint program, const GLchar* uniformBlockName)
{
    auto result = OpenglRedirectorBase::glGetUniformBlockIndex(program, uniformBlockName);
    if(!m_Manager.hasProgram(program))
        return result;
    auto& record = m_Manager.getMutableProgram(program);
    if(record.m_UniformBlocks.count(uniformBlockName) == 0)
    {
        ShaderManager::ShaderProgram::UniformBlock block;
        block.location = result;
        record.m_UniformBlocks[std::string(uniformBlockName)] = block;
    }
    return result;
}

void Repeater::glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) 
{
    OpenglRedirectorBase::glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    if(!m_Manager.hasProgram(program))
        return;

    auto& record = m_Manager.getMutableProgram(program);
    record.updateUniformBlock(uniformBlockIndex, uniformBlockBinding);

    // Add binding index's transformation metadata
    if(record.m_VertexShader != -1)
    {
        const auto& desc = m_Manager.getShaderDescription(record.m_VertexShader);
        if(desc.isUBOused() && record.hasUniformBlock(desc.m_InterfaceBlockName))
        {
            auto& block = record.m_UniformBlocks[desc.m_InterfaceBlockName];
            if(OpenglRedirectorBase::glGetUniformBlockIndex(program,desc.m_InterfaceBlockName.c_str())  == uniformBlockIndex)
            {
                auto& index = m_UniformBlocksTracker.getBindingIndex(block.bindingIndex);

                std::array<const GLchar*, 1> uniformList = {desc.m_TransformationMatrixName.c_str()};
                std::array<GLuint, 1> resultIndex;
                std::array<GLint, 1> params;
                OpenglRedirectorBase::glGetUniformIndices(program, 1, uniformList.data(), resultIndex.data());
                OpenglRedirectorBase::glGetActiveUniformsiv(program, 1, resultIndex.data(), GL_UNIFORM_OFFSET, params.data());

                // Store offset of uniform in block
                index.transformationOffset = params[0];
            }
        }
    }
}

void Repeater::glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    OpenglRedirectorBase::glBindBufferRange(target, index, buffer, offset, size);
    if(target == GL_UNIFORM_BUFFER)
        m_UniformBlocksTracker.setUniformBinding(buffer,index);
}
void Repeater::glBindBufferBase (GLenum target, GLuint index, GLuint buffer)
{
    OpenglRedirectorBase::glBindBufferBase(target, index, buffer);
    if(target == GL_UNIFORM_BUFFER)
        m_UniformBlocksTracker.setUniformBinding(buffer,index);
}

void Repeater::glBindBuffersBase (GLenum target, GLuint first, GLsizei count, const GLuint* buffers)
{
    OpenglRedirectorBase::glBindBuffersBase(target,first, count, buffers);

    if(target == GL_UNIFORM_BUFFER)
    {
        for(size_t i = 0; i < count; i++)
        {
            m_UniformBlocksTracker.setUniformBinding(buffers[i],first+i);
        }
    }
}

void Repeater::glBindBuffersRange (GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes) 
{
    OpenglRedirectorBase::glBindBuffersRange(target,first,count, buffers, offsets, sizes);

    if(target == GL_UNIFORM_BUFFER)
    {
        for(size_t i = 0; i < count; i++)
        {
            m_UniformBlocksTracker.setUniformBinding(buffers[i],first+i);
        }
    }
}


void Repeater::glBufferData (GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    OpenglRedirectorBase::glBufferData(target,size,data,usage);

    if(target != GL_UNIFORM_BUFFER)
        return;

    GLint bufferID = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &bufferID);
    if(!m_UniformBlocksTracker.hasBufferBindingIndex(bufferID))
        return;
    auto index = m_UniformBlocksTracker.getBufferBindingIndex(bufferID);
    auto& metadata = m_UniformBlocksTracker.getBindingIndex(index);
    if(metadata.transformationOffset == -1)
    {
        // Find a program whose uniform iterface contains transformation matrix
        for(auto& [programID, program]: m_Manager.getMutablePrograms())
        {
            auto& blocks = program.m_UniformBlocks;
            
            // Determine fi shader program contains a block, whose binding index is same as
            // currently bound GL_UNIFORM_BUFFER
            auto result = std::find_if(blocks.begin(), blocks.end(), [&](const auto& block)->bool
            {
                return (block.second.bindingIndex == index);
            });
            // If index is 0, than any block would do
            if(result == blocks.end() && index != 0)
                continue;
            // Determine if program's VS has transformation in interface block
            const auto VS = program.m_VertexShader;
            if(!m_Manager.hasShader(VS))
                continue;
            const auto& shader = m_Manager.getShaderDescription(VS);
            if(!shader.isUBOused())
                continue;

            std::array<const GLchar*, 1> uniformList = {shader.m_TransformationMatrixName.c_str()};
            std::array<GLuint, 1> resultIndex;
            std::array<GLint, 1> params;
            OpenglRedirectorBase::glGetUniformIndices(programID, 1, uniformList.data(), resultIndex.data());
            OpenglRedirectorBase::glGetActiveUniformsiv(programID, 1, resultIndex.data(), GL_UNIFORM_OFFSET, params.data());

            // Store offset of uniform in block
            metadata.transformationOffset = params[0];

            // Re-store bindingIndex if needed
            blocks[shader.m_InterfaceBlockName].bindingIndex = index;
        }
    }
    if(metadata.transformationOffset != -1)
    {
        if(size >= metadata.transformationOffset+sizeof(float)*16)
        {
            std::memcpy(glm::value_ptr(metadata.transformation), static_cast<const std::byte*>(data)+metadata.transformationOffset, sizeof(float)*16);
            auto estimatedParameters = estimatePerspectiveProjection(metadata.transformation);

            printf("[Repeater] estimating parameters from UBO\n");
            printf("[Repeater] parameters: fx(%f) fy(%f) near (%f) near(%f) isPerspective (%d) \n", estimatedParameters.fx, estimatedParameters.fy, estimatedParameters.nearPlane, estimatedParameters.farPlane, estimatedParameters.isPerspective);

            // TODO: refactor into class method of Binding index structure
            metadata.projection = estimatedParameters;
            metadata.hasTransformation = true;
        }
    }
}
void Repeater::glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    OpenglRedirectorBase::glBufferSubData(target,offset,size,data);

    if(target != GL_UNIFORM_BUFFER)
        return;

    GLint bufferID = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &bufferID);
    if(!m_UniformBlocksTracker.hasBufferBindingIndex(bufferID))
        return;
    auto index = m_UniformBlocksTracker.getBufferBindingIndex(bufferID);
    auto& metadata = m_UniformBlocksTracker.getBindingIndex(index);
    if(metadata.transformationOffset != -1 && offset <= metadata.transformationOffset)
    {
        if(offset+size >= metadata.transformationOffset+sizeof(float)*16)
        {
            std::memcpy(glm::value_ptr(metadata.transformation), static_cast<const std::byte*>(data)+metadata.transformationOffset, sizeof(float)*16);
            auto estimatedParameters = estimatePerspectiveProjection(metadata.transformation);
            printf("[Repeater] estimating parameters from UBO\n");
            printf("[Repeater] parameters: fx(%f) fy(%f) near (%f) near(%f) \n", estimatedParameters.fx, estimatedParameters.fy, estimatedParameters.nearPlane, estimatedParameters.farPlane);

            metadata.projection = estimatedParameters;
            metadata.hasTransformation = true;
        }
    }
}

// ----------------------------------------------------------------------------
void Repeater::glMatrixMode(GLenum mode)
{
    OpenglRedirectorBase::glMatrixMode(mode);
    m_LegacyTracker.matrixMode(mode);
    //printf("[Repeater] glMatrixMode %s\n", ve::opengl_utils::getEnumStringRepresentation(mode).c_str());
}
void Repeater::glLoadMatrixd(const GLdouble* m)
{
    OpenglRedirectorBase::glLoadMatrixd(m);
    if(m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = helper::createMatrixFromRawGL(m);
        m_LegacyTracker.loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}
void Repeater::glLoadMatrixf(const GLfloat* m)
{
    OpenglRedirectorBase::glLoadMatrixf(m);
    if(m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = helper::createMatrixFromRawGL(m);
        m_LegacyTracker.loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}

void Repeater::glLoadIdentity(void)
{
    OpenglRedirectorBase::glLoadIdentity();
    if(m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        glm::mat4 identity = glm::mat4(1.0);
        m_LegacyTracker.loadMatrix(std::move(identity));
    }
}

void Repeater::glMultMatrixd(const GLdouble* m)
{
    OpenglRedirectorBase::glMultMatrixd(m);
    if(m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = helper::createMatrixFromRawGL(m);
        m_LegacyTracker.loadMatrix(std::move(result));
    }
}

void Repeater::glMultMatrixf(const GLfloat* m)
{
    OpenglRedirectorBase::glMultMatrixf(m);
    if(m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = helper::createMatrixFromRawGL(m);
        m_LegacyTracker.loadMatrix(std::move(result));
    }
}


void Repeater::glOrtho(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val)
{
    OpenglRedirectorBase::glOrtho(left,right,bottom,top,near_val,far_val);
    m_LegacyTracker.multMatrix(glm::ortho(left,right,bottom,top,near_val,far_val));
}

void Repeater::glFrustum(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val)
{
    OpenglRedirectorBase::glFrustum(left,right,bottom,top,near_val,far_val);
    m_LegacyTracker.multMatrix(glm::frustum(left,right,bottom,top,near_val,far_val));
}

void Repeater::glBegin(GLenum mode)
{
    if(m_callList == 0)
    {
        m_callList = glGenLists(1);
    }
    glNewList(m_callList, GL_COMPILE);
    OpenglRedirectorBase::glBegin(mode);
}

void Repeater::glEnd()
{
    OpenglRedirectorBase::glEnd();
    glEndList();

    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glCallList(m_callList);
    });
}

void Repeater::glCallList(GLuint list)
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glCallList(list);
    });
}
void Repeater::glCallLists(GLsizei n,GLenum type,const GLvoid* lists)
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glCallLists(n,type, lists);
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

        const float increment = 0.10f;
        switch(keySym)
        {
            case XK_F1:
            {
                m_cameraParameters.m_XShiftMultiplier += increment;
                puts("[Repeater] Setting: F1 pressed - increase angle");
            }
            break;
            case XK_F2:
            {
                m_cameraParameters.m_XShiftMultiplier -= increment;
                puts("[Repeater] Setting: F2 pressed - decrease angle");
            }
            break;

            case XK_F3:
            {
                m_cameraParameters.m_frontOpticalAxisCentreDistance += 0.5;
                puts("[Repeater] Setting: F3 pressed increase dist");
            }
            break;

            case XK_F4:
            {
                m_cameraParameters.m_frontOpticalAxisCentreDistance -= 0.5;
                puts("[Repeater] Setting: F4 pressed decrease dist");
            }
            break;

            case XK_F5:
            {
                m_cameraParameters = CameraParameters();
                puts("[Repeater] Setting: F5 pressed - reset");
                break;
            }

            case XK_F12:
            case XK_F11:
            {
                m_IsMultiviewActivated = !m_IsMultiviewActivated;
                puts("[Repeater] Setting: F11 pressed - toggle");
            }
            break;
        }
        printf("[Repeater] Setting: frontDistance (%f), X multiplier(%f)\n", 
                m_cameraParameters.m_frontOpticalAxisCentreDistance, m_cameraParameters.m_XShiftMultiplier);
        m_cameras.updateParamaters(m_cameraParameters);
    }
    return returnVal;
}

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

void Repeater::setEnhancerShift(const glm::mat4& viewSpaceTransform, float projectionAdjust)
{
    const auto& resultMat = viewSpaceTransform;
    auto program = m_Manager.getBoundedProgramID();
    if(program)
    {
        auto location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_view_transform");
        OpenglRedirectorBase::glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(glm::value_ptr(resultMat)));

        location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_identity");
        OpenglRedirectorBase::glUniform1i(location, GL_FALSE);

        location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_projection_adjust");
        OpenglRedirectorBase::glUniform1f(location, projectionAdjust);
    }

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
        auto oldProjection = m_LegacyTracker.getProjection();
        oldProjection[2][0] = projectionAdjust;
        const auto newProjection = oldProjection*resultMat;
        if(!m_LegacyTracker.isOrthogonalProjection())
            OpenglRedirectorBase::glLoadMatrixf(glm::value_ptr(newProjection));
    }
}

void Repeater::resetEnhancerShift()
{
    if(m_LegacyTracker.isLegacyNeeded())
    {
        OpenglRedirectorBase::glLoadMatrixf(glm::value_ptr(m_LegacyTracker.getProjection()));
        OpenglRedirectorBase::glMatrixMode(m_LegacyTracker.getMatrixMode());
    }
}

void Repeater::setEnhancerIdentity()
{
    const auto identity = glm::mat4(1.0);
    auto program = m_Manager.getBoundedProgramID();
    auto location = OpenglRedirectorBase::glGetUniformLocation(program, "enhancer_identity");
    OpenglRedirectorBase::glUniform1i(location, GL_TRUE);
}

void Repeater::takeScreenshot(const std::string filename)
{
    const auto width = currentViewport.getWidth();
    const auto height = currentViewport.getHeight();
    // Make the BYTE array, factor of 3 because it's RBG.
    BYTE* pixels = new BYTE[3 * width * height];

    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

    // Convert to FreeImage format & save to file
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0xFF, 0xFF00, 0xFF0000, false);
    if(!FreeImage_Save(FIF_BMP, image, filename.c_str(), 0))
    {
        printf("[Repeater] Failed to save screenshot %s\n", filename.c_str());
    }
    // Free resources
    FreeImage_Unload(image);
    delete [] pixels;
}

void Repeater::duplicateCode(const std::function<void(void)>& code)
{
    bool shouldNotDuplicate = (
            !m_IsMultiviewActivated ||
            // don't duplicate while rendering light's point of view into shadow map
            m_FBOTracker.isFBOshadowMap() ||
            // don't duplicate while there is no projection and its not legacy OpenGL at the same time
            (m_Manager.isAnyBound() && m_Manager.isVSBound() && !m_Manager.getBoundedVS().hasDetectedTransformation() && !m_LegacyTracker.isLegacyNeeded())
    );

    if(shouldNotDuplicate)
    {
        setEnhancerIdentity();
        code();
        return;
    }

    /// If Uniform Buffer Object is used
    if(m_Manager.isAnyBound() && m_Manager.getBoundedVS().isUBOused())
    {
        const auto& blockName = m_Manager.getBoundedVS().m_InterfaceBlockName;
        auto index = m_Manager.getBoundedProgram().m_UniformBlocks[blockName].bindingIndex;
        const auto& indexStructure = m_UniformBlocksTracker.getBindingIndex(index);
        if(indexStructure.hasTransformation)
        {
            setEnhancerDecodedProjection(m_Manager.getBoundedProgramID(),indexStructure.projection);
        } else {
            printf("[Repeater] Unexpected state. Expected UBO, but not found. Falling back to identity\n");
            setEnhancerIdentity();
        }
    }
    // Get original viewport
    auto originalViewport = currentViewport;
    auto originalScissor = currentScissorArea;

    const auto setup = m_cameras.getCameraGridSetup();
    const auto& tilesPerX = setup.first;
    const auto& tilesPerY = setup.second;
    // for each virtual camera, create a subview (subviewport)
    // and render draw call using its transformation into this subview

    size_t cameraID = 0;
    for(const auto& camera: m_cameras.getCameras())
    {
        // If only-camera mode is active && ID does not match
        if(m_diagnostics.shouldShowOnlySpecificVirtualCamera() &&
                cameraID++ != m_diagnostics.getOnlyCameraID())
            continue;
        const auto& currentStartX = camera.getViewport().getX();
        const auto& currentStartY = camera.getViewport().getY();

        const auto scissorDiffX = (originalScissor.getX()-originalViewport.getX())/tilesPerX;
        const auto scissorDiffY = (originalScissor.getY()-originalViewport.getY())/tilesPerY;
        const auto scissorWidth = originalScissor.getWidth()/tilesPerX; 
        const auto scissorHeight = originalScissor.getHeight()/tilesPerY; 
        if(!m_diagnostics.shouldShowOnlySpecificVirtualCamera())
        {
            OpenglRedirectorBase::glScissor(currentStartX+scissorDiffX, currentStartY+scissorDiffY, scissorWidth, scissorHeight);
        }

        // Detect if VS renders into clip-space, thus if z is always 1.0
        // => in such case, we don't want to translate the virtual camera
        bool isClipSpaceRendering = false;
        if(m_Manager.isAnyBound() && m_Manager.isVSBound())
        {
            isClipSpaceRendering = (m_Manager.getBoundedVS().m_IsClipSpaceTransform);
        }
        const auto& t = (isClipSpaceRendering)?camera.getViewMatrixRotational():camera.getViewMatrix();
        const auto& v = camera.getViewport();
        if(!m_diagnostics.shouldShowOnlySpecificVirtualCamera())
        {
            OpenglRedirectorBase::glViewport(v.getX(), v.getY(), v.getWidth(), v.getHeight());
        }
        setEnhancerShift(t,camera.getAngle()*m_cameraParameters.m_XShiftMultiplier/m_cameraParameters.m_frontOpticalAxisCentreDistance);
        code();
        resetEnhancerShift();
    }
    // restore
    OpenglRedirectorBase::glViewport(originalViewport.getX(), originalViewport.getY(), originalViewport.getWidth(), originalViewport.getHeight());
    OpenglRedirectorBase::glScissor(originalScissor.getX(), originalScissor.getY(), originalScissor.getWidth(), originalScissor.getHeight());
}

