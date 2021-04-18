/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        startup_main.cpp
*
*****************************************************************************/

#include "dispatcher.hpp"
#include "startup_injector.hpp"

__attribute((constructor)) void hi_setup()
{
    puts("Injector startup");
    injector_setup(std::make_unique<hi::Dispatcher>());
}

__attribute((destructor)) void hi_cleaner()
{
    puts("Injector cleaning");
    injector_cleaner();
}
