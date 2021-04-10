/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        startup_enhancer.hpp
*
*****************************************************************************/

#ifndef __STARTUP_ENHANCER__HPP
#define __STARTUP_ENHANCER__HPP
#include "hooking/redirector_base.hpp"
#include <memory>
/**
 * @brief Call on process/DLL initialization to set up redirections
 *
 * @param redirector
 */
void enhancer_setup(std::unique_ptr<ve::hooking::RedirectorBase> redirector);
void enhancer_cleaner();

#endif
