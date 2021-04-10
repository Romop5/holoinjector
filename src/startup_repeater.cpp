/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        startup_repeater.cpp
*
*****************************************************************************/

#include "repeater.hpp"
#include "startup_enhancer.hpp"

__attribute((constructor)) void repeater_setup()
{
    puts("Repeater startup");
    enhancer_setup(std::make_unique<ve::Repeater>());
}

__attribute((destructor)) void repeater_cleaner()
{
    puts("Repeater cleaning");
    enhancer_cleaner();
}
