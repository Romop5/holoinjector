/*
 * Requires (as sudo)
 * setsebool allow_execheap on
 */
#include <dlfcn.h>
#include <cstdio>
#include <subhook.h>
#include <mutex>
#include <memory>

#include "redirector_base.hpp"

#include <unistd.h>
#include <cassert>

using namespace ve;

struct EnhancerContext
{
    std::unique_ptr<RedirectorBase> redirector;
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
            puts("[Enhancer] failed to get original address of dlsym\n");
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

        assert(context != nullptr);
        const auto& functions = context->redirector->getRedirectedFunctions();
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
    bool hook_function(subhook_t& hook, void* target, void* ourHandler,subhook_flags_t hookTypeFlags = (subhook_flags_t)(0))
    {
        hook = subhook_new(target, ourHandler, hookTypeFlags);
        auto retval = subhook_install(hook);
        if(retval != 0)
        {
            fputs("[Enhancer] failed: failed to hook dlsym()\n",stdout);
            return false;
        }

        auto trampoline_test = subhook_get_trampoline(hook);
        if(trampoline_test == NULL)
        {
            fputs("[Enhancer] failed: failed to create trampoline()\n",stdout);
            return false;
        }
        return true;
    }

    template<typename DICT>
    void dumpRedirections(DICT dict)
    {
        printf("Dumping redirected functions (%lu):\n", dict.size());
        for(const auto& keyValue: dict)
        {
            puts(keyValue.first.c_str());
        }
        puts("End of dump");
    }
} //namespace helper

/* 
 * Calls dlsym() at least once before hooking. The idea is that dlsym() is called via PLT and GOT,
 * and shared loaders use 'lazy binding', so the real address is rellocated/placed in PLT only
 * after the first call.
 *
 * If we hooked dlsym() without calling it first, the first call to such hooked function would
 * replace/corrupt trampoline code, leading to crash.
 */
void _hack_preventLazyBinding()
{
    auto test = dlsym(reinterpret_cast<void*>(RTLD_DEFAULT), "enhancer_setup");
    static_cast<void>(test);
}

/**
 * @brief Execute when DLL is loaded to process
 *
 * Hooks all neccessary functions to detour OpenGL library calls
 */
void enhancer_setup(std::unique_ptr<RedirectorBase> redirector)
{
    // Necessary hack
    _hack_preventLazyBinding();

    puts("[Enhancer] starting setup...");
    // Decide whether to use 64bit hook or not
    auto hookTypeFlags = static_cast<subhook_flags>(SUBHOOK_BITS == 32?0:SUBHOOK_64BIT_OFFSET);
    context = std::make_unique<EnhancerContext>();
    context->redirector = std::move(redirector);
    /*
     * Hook dlopen/dlsym functions and dispatch via our own function
     */
    auto dlSymHookStatus = helper::hook_function(context->dlsymhook, reinterpret_cast<void *>(dlsym), reinterpret_cast<void *>(hooked_dlsym), hookTypeFlags);
    
    /*
     * Set original symbol getter using trampolined dlsym()
     */
    context->redirector->setSymbolGetter([](const char* symbol)->void* 
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
    context->redirector->registerCallbacks();    
    helper::dumpRedirections(context->redirector->getRedirectedFunctions().getMapping());
    fputs("[Enhancer] Registration done\n",stdout);
    if(dlSymHookStatus == false)
    {
        fputs("[Enhancer] Hooking dlsym() failed -> we can't hook any of OpenGL API functions\n",stdout);
        fputs("[Enhancer] Make sure that 'getsebool allow_execheap' is on\n",stdout);
        fputs("[Enhancer] If not, run 'setsebool allow_execheap on' as administrator\n",stdout);
    }
}

void enhancer_cleaner()
{
    context.reset();
}
