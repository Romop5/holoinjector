#ifndef VE_UTILS_OPENGL_UTILS_HPP
#define VE_UTILS_OPENGL_UTILS_HPP
#include <string>
#include <GL/gl.h>
#include <glm/glm.hpp>

namespace ve 
{
namespace opengl_utils
{
    template<typename T>
    void dumpOpenglMatrix(const T* m);

    glm::mat4 createMatrixFromRawGL(const GLfloat* values);
    glm::mat4 createMatrixFromRawGL(const GLdouble* value);

    /**
     * @brief Convert OpenGL enum 'type' into string representation
     */
    std::string getEnumStringRepresentation(GLenum type);

    bool takeScreenshot(const std::string& path);
} //namespace opengl_utils
} //namespace ve
#endif
