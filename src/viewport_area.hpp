#ifndef ENHANCER_VIEWPORT_AREA_HPP
#define ENHANCER_VIEWPORT_AREA_HPP
#include <GL/gl.h>
namespace ve
{
struct ViewportArea 
{
    GLint data[4];

    ViewportArea();

    GLint* getDataPtr();

    void set(GLint x, GLint y, GLsizei width, GLsizei height);
    
    GLint getX() const;
    GLint getY() const;
    GLint getWidth() const;
    GLint getHeight() const;

    bool operator==(const ViewportArea& area);
}; 
} //namespace ve
#endif
