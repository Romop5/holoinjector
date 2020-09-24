#include <string>
#include <GL/gl.h>

namespace ve 
{
namespace opengl_utils
{
    /**
     * @brief Convert OpenGL enum 'type' into string representation
     */
    std::string getEnumStringRepresentation(GLenum type);
} //namespace opengl_utils
} //namespace ve
