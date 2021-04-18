/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_utils.hpp
*
*****************************************************************************/

#ifndef HI_UTILS_OPENGL_UTILS_HPP
#define HI_UTILS_OPENGL_UTILS_HPP
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace hi
{
namespace opengl_utils
{
    template <typename T>
    void dumpOpenglMatrix(const T* m);

    glm::mat4 createMatrixFromRawGL(const GLfloat* values);
    glm::mat4 createMatrixFromRawGL(const GLdouble* value);

    /**
     * @brief Convert OpenGL enum 'type' into string representation
     */
    std::string getEnumStringRepresentation(GLenum type);

    bool takeScreenshot(const std::string& path, size_t screenWidth, size_t screenHeight);

    std::optional<std::string> getShaderLogMessage(size_t shaderID);
    std::optional<std::string> getProgramLogMessage(size_t programID);

    bool isProgramLinked(size_t programID);
} //namespace opengl_utils
} //namespace hi
#endif
