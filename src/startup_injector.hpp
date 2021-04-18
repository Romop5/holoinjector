/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        startup_injector.hpp
*
*****************************************************************************/

#ifndef __STARTUP_INJECTOR__HPP
#define __STARTUP_INJECTOR__HPP
#include "hooking/redirector_base.hpp"
#include <memory>
/**
 * @brief Call on process/DLL initialization to set up redirections
 *
 * @param redirector
 */
void injector_setup(std::unique_ptr<hi::hooking::RedirectorBase> redirector);
void injector_cleaner();

#endif
