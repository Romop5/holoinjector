/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        managers/shader_manager.cpp
*
*****************************************************************************/

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "context.hpp"
#include "logger.hpp"
#include "diagnostics.hpp"

#include "managers/shader_manager.hpp"
#include "trackers/shader_tracker.hpp"

#include "pipeline/output_fbo.hpp"
#include "pipeline/pipeline_injector.hpp"
#include "utils/glsl_preprocess.hpp"
#include "utils/opengl_utils.hpp"
#include <cmath>

using namespace hi;
using namespace hi::managers;
using namespace hi::pipeline;
using namespace hi::opengl_utils;

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

void dumpCompilationResult(const CompilationResult& result)
{
    if (result.hasLinkedSuccessfully)
    {
        Logger::logDebug("Program creation succeeded");
        return;
    }
    Logger::logDebug("========================================================================");
    Logger::logDebug("START OF COMPILATION RESULT");
    Logger::logDebug("========================================================================");
    Logger::logDebug("Program compilation & link failed. Precise reasons:");
    Logger::logDebug("           Has program linked: ", result.hasLinkedSuccessfully);
    Logger::logDebug("           Link error message ", result.linkErrorMessage.value_or("Unknown"), "\n");
    Logger::logDebug("           Attached shaders", result.shaders.size());

    for (const auto& shader : result.shaders)
    {
        Logger::logDebug("========================================================================");
        Logger::logDebug(" Shader error message ", shader.errorMessage.value_or("Unknown"));
        Logger::logDebug("========================================================================");
        Logger::logDebug(" Shader source code", shader.sourceCode);
        Logger::logDebug("========================================================================");
        Logger::logDebug("END OF SHADER");
        Logger::logDebug("========================================================================");
    }
    Logger::logDebug("========================================================================");
    Logger::logDebug("END OF COMPILATION RESULT");
    Logger::logDebug("========================================================================");
}

CompilationResult tryCompilingShaderProgram(const PipelineInjector::PipelineType pipeline, GLuint programId)
{
    helper::CompilationResult output;
    std::vector<GLuint> newShaderIDs;
    bool hasError = false;
    for (auto& [type, sourceCode] : pipeline)
    {
        ShaderCompilationResult result;
        auto newShader = glCreateShader(type);
        const GLchar* sources[1] = { reinterpret_cast<const GLchar*>(sourceCode.data()) };
        glShaderSource(newShader, 1, sources, nullptr);
        fflush(stdout);
        glCompileShader(newShader);
        GLint status;
        glGetShaderiv(newShader, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE)
        {
            const auto message = getShaderLogMessage(newShader);
            result.errorMessage = message;
            hasError = true;
        }

        result.hasCompiledSuccessfully = status;
        result.sourceCode = sourceCode;
        output.shaders.push_back(result);

        glAttachShader(programId, newShader);
        newShaderIDs.push_back(newShader);
    }

    GLint linkStatus = 0;
    if (!hasError)
    {
        glLinkProgram(programId);
        glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE)
        {
            auto optionalLog = getProgramLogMessage(programId);
            output.linkErrorMessage = optionalLog.value_or("Unable to get log");
            hasError = true;
        }
    }
    for (auto id : newShaderIDs)
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
    auto shaderDesc = std::make_shared<hi::trackers::ShaderMetadata>(id, shaderType);
    context.getManager().shaders.add(id, shaderDesc);
    return id;
}

void ShaderManager::deleteShader(Context& context, GLuint shader)
{
    context.getManager().shaders.remove(shader);
    glDeleteShader(shader);
}

void ShaderManager::shaderSource(Context& context, GLuint shaderId, GLsizei count, const GLchar* const* string, const GLint* length)
{
    auto concatenatedShader = glsl_preprocess::joinGLSLshaders(count, string, length);
    Logger::log("glShaderSource: [", shaderId, "] SOURCE END");
    if (context.getManager().shaders.has(shaderId))
    {
        auto shader = context.getManager().shaders.get(shaderId);
        auto preprocessedShader = glsl_preprocess::preprocessGLSLCode(concatenatedShader);
        shader->preprocessedSourceCode = preprocessedShader;
    }
    std::vector<const char*> shaders = {
        concatenatedShader.c_str(),
    };
    glShaderSource(shaderId, 1, shaders.data(), nullptr);
}

void ShaderManager::linkProgram(Context& context, GLuint programId)
{
    // Link the program for 1st time
    // => we can use native GLSL compiler to detect active uniforms
    //glLinkProgram(programId);
    //
    /* if (context.getDiagnostics().shouldNotBeIntrusive()) */
    /* { */
    /*     glLinkProgram(programId); */
    /*     return; */
    /* } */

    // Note: this should never happen (if we handle all glCreateProgram/Shader)
    assert(context.getManager().has(programId) && "Fatal fail: we missed glCreateShader or it's extension version");
    if (!context.getManager().has(programId))
        return;

    auto program = context.getManager().get(programId);

    /*
     *  Create pipeline injector with correct parameters (number of vies)
     */
    hi::pipeline::PipelineInjector plInjector;
    hi::pipeline::PipelineInjector::PipelineType pipeline;
    hi::pipeline::PipelineParams parameters;

    // TODO: detect if number of invocations is supported
    parameters.countOfInvocations = context.getOutputFBO().getParams().getLayers();
    if (parameters.countOfInvocations > defaultMaximumGSInvocations)
    {
        parameters.countOfPrimitivesDuplicates = floor(parameters.countOfInvocations / defaultMaximumGSInvocations) + 1;
        parameters.countOfInvocations = defaultMaximumGSInvocations;
    }

    /*
     * Deregister all attached shaders (they're going to be changed in any case)
     * and fill pipeline structure out of them
     */
    for (auto [type, shader] : program->shaders.getMap())
    {
        // detach shader from program
        glDetachShader(programId, shader->m_Id);
        // store source code for given shader type
        pipeline[shader->m_Type] = shader->preprocessedSourceCode;
        Logger::log("Detaching:", shader->m_Type, shader->m_Id);
    }

    /*
     * Inject pipeline
     */
    auto resultPipeline = plInjector.process(pipeline, parameters);

    // Use application's original program when in non-intrusive mode
    if (context.getDiagnostics().shouldNotBeIntrusive())
    {
        resultPipeline = plInjector.identity(pipeline);
    }

    program->m_Metadata = std::move(resultPipeline.metadata);
    Logger::log("Pipeline process succeeded?: ", resultPipeline.wasSuccessfull);

    auto status = helper::tryCompilingShaderProgram(resultPipeline.pipeline, programId);
    if (!status.hasLinkedSuccessfully)
    {
        dumpCompilationResult(status);
        // At least compile original pipeline
        auto statusOriginal = helper::tryCompilingShaderProgram(pipeline, programId);
    }

    if (program->m_Metadata)
    {
        program->m_Metadata->m_IsLinkedCorrectly = (status.hasLinkedSuccessfully == GL_TRUE);
    }
}

void ShaderManager::compileShader(Context& context, GLuint shader)
{
    Logger::log("glCompileShader");
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        Logger::logError("Error while comping shader [", shader, "]");
    }
}

void ShaderManager::attachShader(Context& context, GLuint program, GLuint shader)
{
    Logger::log("attaching shader ", shader, " to program ", program);
    //glAttachShader(program,shader);

    if (!context.getManager().has(program) || !context.getManager().shaders.has(shader))
        return;
    context.getManager().get(program)->attachShaderToProgram(context.getManager().shaders.get(shader));
}

GLuint ShaderManager::createProgram(Context& context)
{
    auto result = glCreateProgram();

    auto program = std::make_shared<hi::trackers::ShaderProgram>();
    context.getManager().add(result, program);
    return result;
}

void ShaderManager::deleteProgram(Context& context, GLuint program)
{
    context.getManager().remove(program);
    glDeleteProgram(program);
}

void ShaderManager::useProgram(Context& context, GLuint program)
{
    glUseProgram(program);
    context.getManager().bind(program);
}
