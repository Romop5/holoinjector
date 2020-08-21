#include <subhook.h>

namespace ve
{
    
    void* hooked_dlsym(int params, const char* symbol)
    {

    }
}

/**
 * @brief Execute when DLL is loaded to process
 *
 * Hooks all neccessary functions to detour OpenGL library calls
 */
__attribute((constructor)) void setup()
{
    /*
     * Hook dlopen/dlsym functions and dispatch via our own function
     */
}
