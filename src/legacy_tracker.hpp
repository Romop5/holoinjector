#include <GL/gl.h>
#include <glm/glm.hpp>

namespace ve
{
class LegacyTracker
{
    public:
    /* Query - start */
    bool isLegacyNeeded() const;
    bool isOrthogonalProjection();
    GLenum getCurrentMode() const;
    /* Query - end */
    /// set current mode, affecting operations afterwards
    void matrixMode(GLenum type);
    /// store matrix according to mode
    void loadMatrix(const glm::mat4&& projection);

    const glm::mat4& getProjection() const;
    private:
    GLenum m_currentMode = GL_PROJECTION;
    glm::mat4 m_currentProjection = glm::mat4(1.0);

    bool m_isLegacyOpenGLUsed = false;
    
};

} // namespace ve
