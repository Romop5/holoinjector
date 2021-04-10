/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        main.cpp
*
*****************************************************************************/

#include <dlfcn.h>
#include "GL/gl.h"
#include <iostream>

void glClear(GLbitfield mask)
{
    std::cout << "Hello!"<< std::endl;
}

int main()
{
    std::cout << "Hi, I am a stupid app which does nothing :(" << std::endl;

    auto address = dlsym(RTLD_DEFAULT,"glClear");
    std::cout << "address: " << address << std::endl;
    using clear_type = void (GLbitfield params);
    clear_type* a = reinterpret_cast<clear_type*>(address);
    a(GL_COLOR_BUFFER_BIT);

    std::cout << "terminating " << std::endl;
}
