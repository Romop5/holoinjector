/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        trackers/legacy_tracker.hpp
*
*****************************************************************************/

#include <GL/gl.h>
#include <glm/glm.hpp>

namespace ve
{
namespace trackers
{
    /**
     * \brief Tracks fixed-pipeline methods (such as settings of projects)
     *
     * This class intercepts projection matrix from fixed-pipeline methods of legacy OpenGL
     * programming model.
     */
    class LegacyTracker
    {
        public:
        /* 
         * Queries / heuristics
         */
        /// Determines if any of fixed-pipeline functions has been called since start of program
        bool isLegacyNeeded() const;

        /// Determines if top of projection stack holds an orthogonal projection
        bool isOrthogonalProjection();

        /// Returns intercepted matrix
        const glm::mat4& getProjection() const;
        /*
         * Book-keeping methods
         */
        /// Set current mode, affecting operations afterwards
        void matrixMode(GLenum type);

        /// Return current mode
        GLenum getMatrixMode() const;

        /// Store matrix according to mode
        void loadMatrix(const glm::mat4& m);

        /// Multiply top of stack with m
        void multMatrix(const glm::mat4& m);

        private:
        /// Mode: affects load/multMatrix() operations
        GLenum m_currentMode = GL_PROJECTION;
        
        /// Intercepted projection matrix
        glm::mat4 m_currentProjection = glm::mat4(1.0);

        /// Flag: has any fixed-pipeline method been used
        bool m_isLegacyOpenGLUsed = false;

        /// Cache: store whether least recent GL_PROJECTION is orthogonal
        bool m_isOrthogonalProjection = false;
        
    };

} // namespace trackers
} // namespace ve
