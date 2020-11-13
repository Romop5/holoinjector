#ifndef ENHANCER_VIEWPORT_AREA_HPP
#define ENHANCER_VIEWPORT_AREA_HPP
#include <GL/gl.h>
namespace ve
{
/**
 * @brief Utility for storing & accessing OpenGL's viewport structure with more comfort
 */
class ViewportArea 
{
    public:
    ViewportArea();
    ViewportArea(GLint x, GLint y, GLsizei width, GLsizei height);

    /// Get pointer to underlying data
    GLint* getDataPtr();

    void set(GLint x, GLint y, GLsizei width, GLsizei height);
    
    GLint getX() const;
    GLint getY() const;
    GLint getWidth() const;
    GLint getHeight() const;

    /// Are two areas equal?
    bool operator==(const ViewportArea& area) const;

    /// Get size, UB for index > 3
    GLint operator[](GLsizei index) const;

    private:
    GLint data[4];
}; 
} //namespace ve
#endif