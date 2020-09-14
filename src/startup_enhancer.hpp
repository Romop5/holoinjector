#ifndef __STARTUP_ENHANCER__HPP
#define __STARTUP_ENHANCER__HPP
#include <memory>
#include "redirector_base.hpp"
/**
 * @brief Call on process/DLL initialization to set up redirections
 *
 * @param redirector
 */
void enhancer_setup(std::unique_ptr<ve::RedirectorBase> redirector);
void enhancer_cleaner();

#endif
