#ifndef VE_UTILS_OPENGL_OBJECTS_HPP
#define VE_UTILS_OPENGL_OBJECTS_HPP

#include <vector>
#include <GL/gl.h>

#include "logger.hpp"

namespace ve 
{
namespace utils
{
    class glObject
    {
        public:
        glObject() = default;
        explicit glObject(size_t id): m_ID(id) {}
        glObject(glObject&& o)
        {
            m_ID = o.m_ID;
            o.m_ID = 0;
        }
        ~glObject()
        {
            if(m_ID == 0)
                return;
            if(glIsProgram(m_ID))
                glDeleteProgram(m_ID);
            else if(glIsShader(m_ID))
                glDeleteShader(m_ID);
            m_ID = 0;
        }
        size_t getID() const { return m_ID; }
        size_t releaseID() {
            auto id = m_ID;
            m_ID = 0;
            return id;
        }
        protected:
        size_t m_ID = 0;
        void setID(size_t id) { m_ID = id; }
    };
    class glShader: public glObject
    {
        public:
        glShader() = default;
        glShader(const std::string& src, GLenum type)
        {
            create(src, type);
        }
        bool create(const std::string& src, GLenum type)
        {
            auto shaderId = glCreateShader(type);
            const char* code = src.c_str();
            glShaderSource(shaderId, 1, &code,nullptr);
            glCompileShader(shaderId);
            GLint compileStatus;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
            if(compileStatus != GL_TRUE)
            {
                GLint logSize = 0;
                glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
                
                GLsizei realLogLength = 0;
                GLchar log[5120] = {0,};
                glGetShaderInfoLog(shaderId, logSize, &realLogLength, log);
                Logger::log("[Repeater] Failed to compile {}\n", log);
                Logger::log("[Repeater] code {}\n", src.c_str());
                std::fflush(stdout);
            }
            setID(shaderId);
            assert(compileStatus == GL_TRUE);
            return shaderId;
        }
    };
    class glProgram: public glObject
    {
        public:
        glProgram() = default;

        template<class ...TYPES>
        glProgram(TYPES... types)
        {
            const std::vector<size_t> ids = {types.getID()...};
            link(ids);
        }
        bool link(const std::vector<size_t> shaders)
        {
            auto program = glCreateProgram();

            for(const auto& shader: shaders)
                glAttachShader(program, shader);

            glLinkProgram(program);
            GLint linkStatus;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            assert(linkStatus == GL_TRUE);
            setID(program);
            return true;
        }
        bool isLinked() const { return getID(); }
    };

} //namespace utils
} //namespace ve
#endif
