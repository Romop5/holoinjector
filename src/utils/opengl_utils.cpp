#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include <string>
#include <unordered_map>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "utils/opengl_utils.hpp"
#include "logger.hpp"

#include "FreeImage.h"

template<typename T>
void ve::opengl_utils::dumpOpenglMatrix(const T* m)
{
    Logger::log("Matrix: ");
    Logger::log("", m[0],m[4],m[8],m[12]);
    Logger::log("", m[1],m[5],m[9],m[13]);
    Logger::log("", m[2],m[6],m[10],m[14]);
    Logger::log("", m[3],m[7],m[11],m[15]);
}

template<>
void ve::opengl_utils::dumpOpenglMatrix<glm::mat4>(const glm::mat4* m);

glm::mat4 ve::opengl_utils::createMatrixFromRawGL(const GLfloat* values)
{
    glm::mat4 result;
    std::memcpy(glm::value_ptr(result), values, 16*sizeof(GLfloat));
    return result;
}

glm::mat4 ve::opengl_utils::createMatrixFromRawGL(const GLdouble* value)
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

std::string ve::opengl_utils::getEnumStringRepresentation(GLenum type)
{
    return "UNKNOWN";
}

bool ve::opengl_utils::takeScreenshot(const std::string& path)
{
    using BYTE = uint8_t;
    GLfloat viewport[4];
    glGetFloatv(GL_VIEWPORT,viewport);

    const auto width = size_t(viewport[2]);
    const auto height = size_t(viewport[3]);
    // Make the BYTE array, factor of 3 because it's RBG.
    BYTE* pixels = new BYTE[3 * width * height];

    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

    // Convert to FreeImage format & save to file
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0xFF, 0xFF00, 0xFF0000, false);
    if(!FreeImage_Save(FIF_BMP, image, path.c_str(), 0))
    {
        Logger::logError("Failed to save screenshot:", path.c_str());
        return false;
    }
    // Free resources
    FreeImage_Unload(image);
    delete [] pixels;
    return true;
}

std::optional<std::string> ve::opengl_utils::getShaderLogMessage(size_t shaderID)
{
    GLint logSize = 0;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logSize);
    if(logSize)
    {
        std::string log(logSize+1, ' ');
        GLsizei realLogLength = 0;
        glGetShaderInfoLog(shaderID, logSize, &realLogLength, log.data());
        if(realLogLength)
        {
            return log;
        }
    }
    return {};
}

std::optional<std::string> ve::opengl_utils::getProgramLogMessage(size_t programID)
{
    GLint logSize = 0;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logSize);
    if(logSize)
    {
        std::string log(logSize+1, ' ');
        GLsizei realLogLength = 0;
        glGetProgramInfoLog(programID, logSize, &realLogLength, log.data());
        if(realLogLength)
        {
            return log;
        }
    }
    return {};
}

bool ve::opengl_utils::isProgramLinked(size_t programID)
{
    GLint linkStatus = 0;
    glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
    return linkStatus;
}
