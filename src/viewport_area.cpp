#include "viewport_area.hpp"
#include <cstdlib> // size_t
#include <cstring> // memcmp

using namespace ve;

ViewportArea::ViewportArea()
{
    for(size_t i = 0; i< 3;i++)
        data[i] = 0;
}

GLint* ViewportArea::getDataPtr() 
{
    return data;
}
void ViewportArea::set(GLint x, GLint y, GLsizei width, GLsizei height)
{
    this->data[0] = x;
    this->data[1] = y;
    this->data[2] = width;
    this->data[3] = height;
}

GLint ViewportArea::getX() const { return data[0]; }
GLint ViewportArea::getY() const { return data[1]; }
GLint ViewportArea::getWidth() const { return data[2]; }
GLint ViewportArea::getHeight() const { return data[3]; }

bool ViewportArea::operator==(const ViewportArea& area)
{
    return std::memcmp(data, area.data, 4*sizeof(GLint)) == 0;
}

