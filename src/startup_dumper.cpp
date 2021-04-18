/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        startup_dumper.cpp
*
*****************************************************************************/

#include "shader_dumper.hpp"
#include "startup_enhancer.hpp"

__attribute((constructor)) void repeater_setup()
{
    puts("Shader dumper startup");
    injector_setup(std::make_unique<hi::ShaderDumper>());
}

__attribute((destructor)) void repeater_cleaner()
{
    injector_cleaner();
}
