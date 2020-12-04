#ifndef VE_UTILS_OPENGL_OBJECTS_HPP
#define VE_UTILS_OPENGL_OBJECTS_HPP

#include <vector>

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glext.h"

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
        glObject& operator=(glObject&& o)
        {
            m_ID = o.m_ID;
            o.m_ID = 0;
            return *this;
        }
        ~glObject()
        {
            if(m_ID == 0)
                return;
            if(glIsProgram(m_ID))
                glDeleteProgram(m_ID);
            else if(glIsShader(m_ID))
                glDeleteShader(m_ID);
            else if(glIsVertexArray(m_ID))
            {
                GLuint id = m_ID;
                glDeleteVertexArrays(1,&id);
            }
            else if(glIsBuffer(m_ID))
            {
                GLuint id = m_ID;
                glDeleteBuffers(1, &id);
            }
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

    class glFullscreenVAO: public glObject
    {
        public:
        glFullscreenVAO()
        {
            struct VertexData
            {
                float position[3];
                float uv[2];
            };
            VertexData vertices[] = 
            {
                {-1.0, -1.0, 0.0, 0.0,0.0},
                {1.0, -1.0, 0.0, 1.0,0.0},
                {-1.0, 1.0, 0.0, -1.0,0.0},
                {1.0, 1.0, 0.0, 1.0,1.0},
            };

            GLuint vertexBuffer;
            glGenBuffers(1, &vertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*5*4, vertices, GL_STATIC_DRAW);

            GLint oldVao;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);
            GLint oldVBO;
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldVBO);

            GLuint vao = 0;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            // Enable coords
            glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(float),0);
            glEnableVertexAttribArray(0);
            // Enable UV
            glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(float),reinterpret_cast<void*>(2*sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, oldVBO);
            glBindVertexArray(oldVao);

            m_VBO = std::move(glObject(vertexBuffer));
            setID(vao);
        }
        GLuint getVAO() const
        {
            return getID();
        }

        void draw()
        {
            GLint oldVao;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);
            glBindVertexArray(getID());   
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(oldVao);
        }
        private:
        glObject m_VBO;
    };

} //namespace utils
} //namespace ve
#endif