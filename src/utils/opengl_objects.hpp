/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_objects.hpp
*
*****************************************************************************/

#ifndef HI_UTILS_OPENGL_OBJECTS_HPP
#define HI_UTILS_OPENGL_OBJECTS_HPP

#include <vector>

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glext.h"

#include "logger.hpp"

namespace hi
{
namespace utils
{
    class glObject
    {
    public:
        glObject() = default;
        explicit glObject(size_t id)
            : m_ID(id)
        {
        }
        glObject(const glObject& o) = delete;
        glObject(glObject&& o)
        {
            m_ID = o.m_ID;
            o.m_ID = 0;
        }
        glObject& operator=(glObject&& o)
        {
            m_ID = o.m_ID;
            o.m_ID = 0;
            return *this;
        }
        ~glObject();
        size_t getID() const { return m_ID; }
        size_t releaseID()
        {
            auto id = m_ID;
            m_ID = 0;
            return id;
        }

    protected:
        size_t m_ID = 0;
        void setID(size_t id) { m_ID = id; }
    };
    class glShader : public glObject
    {
    public:
        glShader() = default;
        glShader(const std::string& src, GLenum type)
        {
            create(src, type);
        }
        bool create(const std::string& src, GLenum type);
    };
    class glProgram : public glObject
    {
    public:
        glProgram() = default;

        template <class... TYPES>
        glProgram(TYPES... types)
        {
            const std::vector<size_t> ids = { types.getID()... };
            link(ids);
        }
        bool link(const std::vector<size_t> shaders);
        bool isLinked() const { return getID(); }
    };

    class glFullscreenVAO : public glObject
    {
    public:
        glFullscreenVAO()
        {
            create();
        }
        void create();
        GLuint getVAO() const
        {
            return getID();
        }

        void draw();

    private:
        glObject m_VBO;
    };

} //namespace utils
} //namespace hi
#endif
