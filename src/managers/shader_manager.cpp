#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "context.hpp"
#include "logger.hpp"
#include "trackers/shader_tracker.hpp"
#include "managers/shader_manager.hpp"

#include <cmath>
#include "utils/glsl_preprocess.hpp"
#include "utils/opengl_utils.hpp"
#include "pipeline/pipeline_injector.hpp"
#include "pipeline/output_fbo.hpp"

using namespace ve::managers;
using namespace ve::pipeline;
using namespace ve::opengl_utils;

/// Default maximal Geometry Shader invocations, defined by OpenGL standard
constexpr auto defaultMaximumGSInvocations = 32;
constexpr auto maximalOpenGLLogSize = 2048;

namespace helper
{
    struct ShaderCompilationResult
    {
        bool hasCompiledSuccessfully;
        std::string sourceCode;
        std::optional<std::string> errorMessage;
    };

    struct CompilationResult
    {
        bool hasLinkedSuccessfully;
        std::vector<ShaderCompilationResult> shaders;
        std::optional<std::string> linkErrorMessage;
    };

    CompilationResult tryCompilingShaderProgram(const PipelineInjector::PipelineType pipeline, GLuint programId)
    {
        helper::CompilationResult output;
        std::vector<GLuint> newShaderIDs;
        bool hasError = false;
        for(auto& [type, sourceCode]: pipeline)
        {
            auto newShader = glCreateShader(type);
            const GLchar* sources[1] = {reinterpret_cast<const GLchar*>(sourceCode.data())};
            glShaderSource(newShader, 1, sources , nullptr);
            fflush(stdout);
            glCompileShader(newShader);
            GLint status;
            glGetShaderiv(newShader,GL_COMPILE_STATUS, &status);
            if(status == GL_FALSE)
            {
                const auto message = getShaderLogMessage(newShader);
                hasError = true;
            }
            glAttachShader(programId, newShader);
            newShaderIDs.push_back(newShader);
        }

        GLint linkStatus = 0;
        if(!hasError)
        {
            glLinkProgram(programId);
            glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
            if(linkStatus == GL_FALSE)
            {
                auto optionalLog = getProgramLogMessage(programId);
                output.linkErrorMessage = (optionalLog ? optionalLog.value() : std::string("Unable to get log"));
                hasError = true;
            }
        }
        for(auto id: newShaderIDs)
        {
            // Detach shaders from program when linking/compilation fails
            glDetachShader(programId, id);
            // Delete shader
            glDeleteShader(id);
        }
        output.hasLinkedSuccessfully = (linkStatus != GL_FALSE && hasError == false);
        return output;
    }
}

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
    assert(context.getManager().has(programId) && "Fatal fail: we missed glCreateShader or it's extension version");
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
    if(parameters.countOfInvocations > defaultMaximumGSInvocations)
    {
        parameters.countOfPrimitivesDuplicates = floor(parameters.countOfInvocations/defaultMaximumGSInvocations)+1;
        parameters.countOfInvocations = defaultMaximumGSInvocations;
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
    program->m_Metadata->m_IsLinkedCorrectly = false;
    Logger::log("[Repeater] Pipeline process succeeded?: ", resultPipeline.wasSuccessfull);

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
            const auto optionalLog = getShaderLogMessage(newShader);
            const auto outputLog = (optionalLog ? optionalLog.value() : "Unable to get log message");
            Logger::logError("[Repeater] Error while compiling new shader type", type, outputLog);
            Logger::logError("Shader source:", sourceCode.c_str());
            return;
        }
        glAttachShader(programId, newShader);
    }
    Logger::log("[Repeater] Relink program with new shaders");
    glLinkProgram(programId);
    auto linkStatus = isProgramLinked(programId);
    if(linkStatus == GL_FALSE)
    {
        const auto optionalLog = getProgramLogMessage(programId);
        const auto outputLog = (optionalLog ? optionalLog.value() : "Unable to get log message");
        // Dump shaders
        Logger::logError("[Repeater] Link failed, dumping shader codes\nDUMP_START\n");
        for(auto& [type, sourceCode]: resultPipeline.pipeline)
        {
            Logger::logError("Shader source:", sourceCode.c_str(), "END_OF_SHADER");
        }
        Logger::logError("[Repeater] Link failed with log:", outputLog, "\nEND_OF_DUMP");
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
