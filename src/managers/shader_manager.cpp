#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "context.hpp"
#include "logger.hpp"
#include "trackers/shader_tracker.hpp"
#include "managers/shader_manager.hpp"

#include <cmath>
#include "utils/glsl_preprocess.hpp"
#include "pipeline/pipeline_injector.hpp"
#include "pipeline/output_fbo.hpp"

using namespace ve::managers;

GLuint ShaderManager::createShader(Context& context, GLenum shaderType)
{
    auto id = glCreateShader(shaderType);
    auto shaderDesc = std::make_shared<ve::trackers::ShaderMetadata>(id,shaderType);
    context.getManager().shaders.add(id, shaderDesc);
    return id;
}

void ShaderManager::shaderSource (Context& context, GLuint shaderId, GLsizei count, const GLchar* const*string, const GLint* length)
{
    auto concatenatedShader = glsl_preprocess::joinGLSLshaders(count, string, length);
    Logger::log("[Repeater] glShaderSource: [",shaderId,"] SOURCE END");
    if(context.getManager().shaders.has(shaderId))
    {
        auto shader = context.getManager().shaders.get(shaderId);
        auto preprocessedShader = glsl_preprocess::preprocessGLSLCode(concatenatedShader);
        shader->preprocessedSourceCode = preprocessedShader;
    }
    std::vector<const char*> shaders = {concatenatedShader.c_str(),};
    glShaderSource(shaderId,1,shaders.data(),nullptr);
}

void ShaderManager::linkProgram (Context& context, GLuint programId)
{
    // Link the program for 1st time
    // => we can use native GLSL compiler to detect active uniforms
    //glLinkProgram(programId);

    // Note: this should never happen (if we handle all glCreateProgram/Shader)
    if(!context.getManager().has(programId))
        return;

    auto program = context.getManager().get(programId);

    /*
     *  Create pipeline injector with correct parameters (number of vies)
     */
    ve::pipeline::PipelineInjector plInjector;
    ve::pipeline::PipelineInjector::PipelineType pipeline;
    ve::pipeline::PipelineParams parameters;

    // TODO: detect if number of invocations is supported
    parameters.countOfInvocations = context.getOutputFBO().getParams().getLayers();
    if(parameters.countOfInvocations > 32)
    {
        parameters.countOfPrimitivesDuplicates = floor(parameters.countOfInvocations/32)+1;
        parameters.countOfInvocations = 32;
    }

    /*
     * Deregister all attached shaders (they're going to be changed in any case)
     * and fill pipeline structure out of them
     */
    for(auto [type,shader]: program->shaders.getMap())
    {
        // detach shader from program
        glDetachShader(programId, shader->m_Id);
        // store source code for given shader type
        pipeline[shader->m_Type] = shader->preprocessedSourceCode;
        Logger::log("[Repeater] Detaching:", shader->m_Type, shader->m_Id);
    }

    /*
     * Inject pipeline
     */
    auto resultPipeline = plInjector.process(pipeline,parameters);
    program->m_Metadata = std::move(resultPipeline.metadata);

    for(auto& [type, sourceCode]: resultPipeline.pipeline)
    {
        auto newShader = glCreateShader(type);
        const GLchar* sources[1] = {reinterpret_cast<const GLchar*>(sourceCode.data())}; 
        glShaderSource(newShader, 1, sources , nullptr);
	Logger::log("[Repeater] Compiling shader:", sourceCode.c_str());
        fflush(stdout);
        glCompileShader(newShader);
        GLint status;
        glGetShaderiv(newShader,GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
        {
            GLint logSize = 0;
            glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &logSize);
            
            GLsizei realLogLength = 0;
            GLchar log[5120] = {0,};
            glGetShaderInfoLog(newShader, logSize, &realLogLength, log);
            Logger::logError("[Repeater] Error while compiling new shader type", type,log);
            Logger::logError("Shader source:", sourceCode.c_str());
            return;
        }
        glAttachShader(programId, newShader);
    }
    Logger::log("[Repeater] Relink program with new shaders");
    glLinkProgram(programId);
    GLint linkStatus = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
    {
        GLint logSize = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logSize);

        GLsizei realLogLength = 0;
        GLchar log[5120] = {0,};
        glGetProgramInfoLog(programId, logSize, &realLogLength, log);

        // Dump shaders
        for(auto& [type, sourceCode]: resultPipeline.pipeline)
        {
            Logger::logError("Shader source:", sourceCode.c_str(), "END_OF_SHADER");
        }
        Logger::logError("[Repeater] Link failed with log:",log);
    }
    assert(linkStatus == GL_TRUE);
    if(program->m_Metadata)
    {
        program->m_Metadata->m_IsLinkedCorrectly = (linkStatus == GL_TRUE);
    }
}

void ShaderManager::compileShader (Context& context, GLuint shader)
{
    Logger::log("[Repeater] glCompileShader");
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS,&status);
    if(status == GL_FALSE)
    {
        Logger::logError("[Repeater] Error while comping shader [", shader, "]");
    }
}

void ShaderManager::attachShader (Context& context, GLuint program, GLuint shader)
{
    Logger::log("[Repeater] attaching shader ",shader," to program ", program);
    //glAttachShader(program,shader);

    if(!context.getManager().has(program) || !context.getManager().shaders.has(shader))
        return;
    context.getManager().get(program)->attachShaderToProgram(context.getManager().shaders.get(shader));
}

GLuint ShaderManager::createProgram (Context& context)
{
    auto result = glCreateProgram();

    auto program = std::make_shared<ve::trackers::ShaderProgram>();
    context.getManager().add(result, program);
    return result;
}

void ShaderManager::useProgram (Context& context, GLuint program)
{
    glUseProgram(program);
    context.getManager().bind(program);
}
