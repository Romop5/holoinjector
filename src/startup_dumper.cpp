/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        startup_dumper.cpp
*
*****************************************************************************/

#include "startup_enhancer.hpp"
#include "shader_dumper.hpp"

__attribute((constructor)) void repeater_setup()
{
    puts("Shader dumper startup");
    enhancer_setup(std::make_unique<ve::ShaderDumper>());
}

__attribute((destructor)) void repeater_cleaner()
{
    enhancer_cleaner();
}
