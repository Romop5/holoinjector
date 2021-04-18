/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_utils.cpp
*
*****************************************************************************/

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include <string>
#include <unordered_map>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "logger.hpp"
#include "utils/opengl_utils.hpp"

#include "FreeImage.h"

template <typename T>
void hi::opengl_utils::dumpOpenglMatrix(const T* m)
{
    Logger::log("Matrix: ");
    Logger::log("", m[0], m[4], m[8], m[12]);
    Logger::log("", m[1], m[5], m[9], m[13]);
    Logger::log("", m[2], m[6], m[10], m[14]);
    Logger::log("", m[3], m[7], m[11], m[15]);
}

template <>
void hi::opengl_utils::dumpOpenglMatrix<glm::mat4>(const glm::mat4* m);

glm::mat4 hi::opengl_utils::createMatrixFromRawGL(const GLfloat* values)
{
    glm::mat4 result;
    std::memcpy(glm::value_ptr(result), values, 16 * sizeof(GLfloat));
    return result;
}

glm::mat4 hi::opengl_utils::createMatrixFromRawGL(const GLdouble* value)
{
    GLfloat newM[16];
    for (size_t i = 0; i < 16; i++)
    {
        newM[i] = static_cast<float>(value[i]);
    }
    glm::mat4 result;
    std::memcpy(glm::value_ptr(result), newM, 16 * sizeof(GLfloat));
    return result;
}

std::string hi::opengl_utils::getEnumStringRepresentation(GLenum type)
{
    return "UNKNOWN";
}

bool hi::opengl_utils::takeScreenshot(const std::string& path, size_t screenWidth, size_t screenHeight)
{
    using BYTE = uint8_t;
    GLfloat viewport[4];
    glGetFloatv(GL_VIEWPORT, viewport);

    // Make the BYTE array, factor of 3 because it's RBG.
    BYTE* pixels = new BYTE[3 * screenWidth * screenHeight];

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glReadPixels(0, 0, screenWidth, screenHeight, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    auto errorStatus = glGetError();
    if (errorStatus != GL_NO_ERROR)
    {
        Logger::logError("Failed to read framebuffer for screen (glReadPixels failed)");
        return false;
    }

    // Convert to FreeImage format & save to file
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, screenWidth, screenHeight, 3 * screenWidth, 24, 0xFF, 0xFF00, 0xFF0000, false);
    if (!FreeImage_Save(FIF_BMP, image, path.c_str(), 0))
    {
        Logger::logError("Failed to save screenshot:", path.c_str());
        return false;
    }
    Logger::log("Saved screenshot:", path.c_str());
    // Free resources
    FreeImage_Unload(image);
    delete[] pixels;
    return true;
}

std::optional<std::string> hi::opengl_utils::getShaderLogMessage(size_t shaderID)
{
    GLint logSize = 0;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize)
    {
        std::string log(logSize + 1, ' ');
        GLsizei realLogLength = 0;
        glGetShaderInfoLog(shaderID, logSize, &realLogLength, log.data());
        if (realLogLength)
        {
            return log;
        }
    }
    return {};
}

std::optional<std::string> hi::opengl_utils::getProgramLogMessage(size_t programID)
{
    GLint logSize = 0;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize)
    {
        std::string log(logSize + 1, ' ');
        GLsizei realLogLength = 0;
        glGetProgramInfoLog(programID, logSize, &realLogLength, log.data());
        if (realLogLength)
        {
            return log;
        }
    }
    return {};
}

bool hi::opengl_utils::isProgramLinked(size_t programID)
{
    GLint linkStatus = 0;
    glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
    return linkStatus;
}
