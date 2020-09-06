/*
 * Requires (as sudo)
 * setsebool allow_execheap on
 */
#include <dlfcn.h>
#include <cstdio>
#include <subhook.h>
#include <mutex>
#include <memory>

#include "repeater.hpp"

using namespace ve;

struct EnhancerContext
{
    Repeater repeater;
    subhook_t dlsymhook;
};

static std::unique_ptr<EnhancerContext> context = nullptr;

namespace ve
{
    void* original_dlsym(void* params, const char* symbol)
    {
        void* original_dlsym = reinterpret_cast<void*>(&dlsym);
        /*
         * Prepare call to original dlsym code
         */
        if(context->dlsymhook)
            original_dlsym = subhook_get_trampoline(context->dlsymhook);
        if(original_dlsym == nullptr)
        {
            puts("Enhancer faield to get original address of dlsym\n");
            exit(1);
        }
        using dlsym_type = void*(void*, const char*);
        return reinterpret_cast<dlsym_type*>(original_dlsym)(params, symbol);
    }
    void* hooked_dlsym(void* params, const char* symbol)
    {
        static std::mutex dlsym_mutex;
        auto lock = std::unique_lock<std::mutex>(dlsym_mutex);

        printf("[Enhancer dlsym] '%s'\n", symbol);

        const auto& functions = context->repeater.getRedirectedFunctions();
        if(functions.hasRedirection(symbol))
        {
            auto* const targetAddress = functions.getTarget(symbol);
            return targetAddress;
        }
        // Else, return original address
        return original_dlsym(params, symbol);
    }

    }

namespace helper
{
    void hook_function(subhook_t& hook, void* target, void* ourHandler,subhook_flags_t hookTypeFlags = (subhook_flags_t)(0))
    {
        hook = subhook_new(target, ourHandler, hookTypeFlags);
        auto retval = subhook_install(hook);
        if(retval != 0)
        {
            fputs("Enhancer failed: failed to hook dlsym()\n",stdout);
            return;
        }

        auto trampoline_test = subhook_get_trampoline(hook);
        if(trampoline_test == NULL)
        {
            fputs("Enhancer failed: failed to create trampoline()\n",stdout);
            return;
        }
    }
} //namespace helper


/**
 * @brief Execute when DLL is loaded to process
 *
 * Hooks all neccessary functions to detour OpenGL library calls
 */
__attribute((constructor)) void enhancer_setup()
{
    // Decide whether to use 64bit hook or not
    auto hookTypeFlags = static_cast<subhook_flags>(SUBHOOK_BITS == 32?0:SUBHOOK_64BIT_OFFSET);
    context = std::make_unique<EnhancerContext>();
    /*
     * Hook dlopen/dlsym functions and dispatch via our own function
     */
    helper::hook_function(context->dlsymhook, reinterpret_cast<void *>(dlsym), reinterpret_cast<void *>(hooked_dlsym), hookTypeFlags);
    
    /*
     * Set original symbol getter using trampolined dlsym()
     */
    context->repeater.setSymbolGetter([](const char* symbol)->void* 
    {
        printf("[Enhancer- symbol getter] Calling original dlsym with symbo %s\n",symbol);
        auto addr = ve::original_dlsym(RTLD_NEXT,symbol);
        if(addr == NULL)
        {
            puts("[Enhancer- symbol getter] Failed to get original address via dlsym()");
        }
        return addr;
    });    

    /*
     * Register OpenGL calls that should be redirected
     */
    context->repeater.registerCallbacks();    
    fputs("[Enhancer] Registration done\n",stdout);
}

__attribute((destructor)) void enhancer_cleaner()
{
    context.reset();
}
