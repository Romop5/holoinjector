/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        startup_injector.cpp
*
*****************************************************************************/

/*
 * Requires (as sudo)
 * setsebool allow_execheap on
 */
#include <cstdio>
#include <dlfcn.h>
//#include <subhook.h>
#include <memory>
#include <mutex>

#include "hooking/redirector_base.hpp"

#include <cassert>
#include <unistd.h>

#define INJECTOR_API_EXPORT __attribute__((visibility("default")))

using namespace hi;

struct InjectorContext
{
    std::unique_ptr<hi::hooking::RedirectorBase> redirector;
    //subhook_t dlsymhook;
};

static std::unique_ptr<InjectorContext> context = nullptr;

extern "C" void* __libc_dlopen_mode(const char* filename, int flag);
extern "C" void* __libc_dlsym(void* handle, const char* symbol);

namespace helper
{
bool shouldLogApiCall()
{
    static bool shouldLogApiCall = false;
    static bool isQueried = false;
    if (!isQueried)
    {
        shouldLogApiCall = (getenv("INJECTOR_LOG_LOAD") != nullptr);
        isQueried = true;
    }
    return shouldLogApiCall;
}

}
namespace hi
{
/*
     * Stores function names, that were replaced by our symbols
     */
std::unordered_map<std::string, void*> m_OriginalCalls;

void* original_dlsym(void* params, const char* symbol)
{
    typedef void* (*PFN_DLSYM)(void*, const char*);
    static PFN_DLSYM dlsym_sym = NULL;
    if (!dlsym_sym)
    {
        void* libdl_handle = __libc_dlopen_mode("libdl.so.2", RTLD_LOCAL | RTLD_NOW);
        if (libdl_handle)
        {
            dlsym_sym = (PFN_DLSYM)__libc_dlsym(libdl_handle, "dlsym");
        }
        if (!dlsym_sym)
        {
            if (helper::shouldLogApiCall())
            {
                printf("[Injector] error: failed to look up real dlsym\n");
            }
            return NULL;
        }
    }
    return dlsym_sym(params, symbol);
    /*
         * SUBHOOK
        void* original_dlsym = reinterpret_cast<void*>(&dlsym);
         *
         * Prepare call to original dlsym code
         *
        if(context->dlsymhook)
            original_dlsym = subhook_get_trampoline(context->dlsymhook);
        if(original_dlsym == nullptr)
        {
            puts("[Injector] failed to get original address of dlsym\n");
            exit(1);
        }
        using dlsym_type = void*(void*, const char*);
        return reinterpret_cast<dlsym_type*>(original_dlsym)(params, symbol);
        */
}
void* hooked_dlsym(void* params, const char* symbol)
{
    /*
         * Fix: route dlopen() directly to libc.so
         * This is needed for libraries such as apitrace
         */
    if (std::string(symbol) == "dlopen")
    {
        //auto moduleHandle = dlopen("libc.so",RTLD_LAZY);
        //return original_dlsym(RTLD_NEXT, symbol);

        typedef void* (*PFN_DLOPEN)(const char*, int);
        static PFN_DLOPEN dlopen_sym = NULL;
        if (!dlopen_sym)
        {
            void* libdl_handle = __libc_dlopen_mode("libdl.so.2", RTLD_LOCAL | RTLD_NOW);
            if (libdl_handle)
            {
                dlopen_sym = (PFN_DLOPEN)__libc_dlsym(libdl_handle, "dlopen");
            }
            if (!dlopen_sym)
            {
                printf("[Injector] error: failed to look up real dlsym\n");
                return NULL;
            }
        }
        return (void*)dlopen_sym;
    }

    static std::mutex dlsym_mutex;
    auto lock = std::unique_lock<std::mutex>(dlsym_mutex);

    if (helper::shouldLogApiCall())
    {
        printf("[Injector dlsym] '%s'\n", symbol);
    }

    assert(context != nullptr);
    const auto& functions = context->redirector->getRedirectedFunctions();

    auto originalSymbol = original_dlsym(params, symbol);
    m_OriginalCalls[symbol] = originalSymbol;

    if (functions.hasRedirection(symbol))
    {
        auto* const targetAddress = functions.getTarget(symbol);
        return targetAddress;
    }
    // Else, return original address
    return originalSymbol;
}

void* getOriginalCallAddress(std::string symbol)
{
    if (m_OriginalCalls.count(symbol) > 0)
    {
        return m_OriginalCalls[symbol];
    }

    if (helper::shouldLogApiCall())
    {
        printf("[Injector- symbol getter] Calling original dlsym with symbo %s\n", symbol.c_str());
    }
    auto addr = hi::original_dlsym(RTLD_NEXT, symbol.c_str());
    if (addr == NULL)
    {
        puts("[Injector- symbol getter] Failed to get original address via dlsym()");
    }
    return addr;
}

} // namespace hi

// Hooked
INJECTOR_API_EXPORT void* dlsym(void* params, const char* symbol)
{
    if (context == nullptr)
        return hi::original_dlsym(params, symbol);
    return hi::hooked_dlsym(params, symbol);
}

namespace helper
{
/* SUBHOOK
    bool hook_function(subhook_t& hook, void* target, void* ourHandler,subhook_flags_t hookTypeFlags = (subhook_flags_t)(0))
    {
        hook = subhook_new(target, ourHandler, hookTypeFlags);
        auto retval = subhook_install(hook);
        if(retval != 0)
        {
            fputs("[Injector] failed: failed to hook dlsym()\n",stdout);
            return false;
        }

        auto trampoline_test = subhook_get_trampoline(hook);
        if(trampoline_test == NULL)
        {
            fputs("[Injector] failed: failed to create trampoline()\n",stdout);
            return false;
        }
        return true;
    }
    */

template <typename DICT>
void dumpRedirections(DICT dict)
{
    printf("Dumping redirected functions (%lu):\n", dict.size());
    for (const auto& keyValue : dict)
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
    auto test = dlsym(reinterpret_cast<void*>(RTLD_DEFAULT), "injector_setup");
    static_cast<void>(test);
}

/**
 * @brief Execute when DLL is loaded to process
 *
 * Hooks all neccessary functions to detour OpenGL library calls
 */
void injector_setup(std::unique_ptr<hi::hooking::RedirectorBase> redirector)
{
    // Necessary hack
    _hack_preventLazyBinding();

    puts("[Injector] starting setup...");
    // Decide whether to use 64bit hook or not
    /* SUBHOOK
     * auto hookTypeFlags = static_cast<subhook_flags>(SUBHOOK_BITS == 32?0:SUBHOOK_64BIT_OFFSET);
     */
    context = std::make_unique<InjectorContext>();
    context->redirector = std::move(redirector);
    /*
     * Hook dlopen/dlsym functions and dispatch via our own function
     */
    //auto dlSymHookStatus = helper::hook_function(context->dlsymhook, reinterpret_cast<void *>(dlsym), reinterpret_cast<void *>(hooked_dlsym), hookTypeFlags);
    auto dlSymHookStatus = true;

    /*
     * Set original symbol getter using trampolined dlsym()
     */
    context->redirector->setSymbolGetter([](const char* symbol) -> void* {
        return getOriginalCallAddress(symbol);
    });

    /*
     * Register OpenGL calls that should be redirected
     */
    context->redirector->registerCallbacks();
    helper::dumpRedirections(context->redirector->getRedirectedFunctions().getMapping());
    fputs("[Injector] Registration done\n", stdout);
    if (dlSymHookStatus == false)
    {
        fputs("[Injector] Hooking dlsym() failed -> we can't hook any of OpenGL API functions\n", stdout);
        fputs("[Injector] Make sure that 'getsebool allow_execheap' is on\n", stdout);
        fputs("[Injector] If not, run 'setsebool allow_execheap on' as administrator\n", stdout);
    }
}

void injector_cleaner()
{
    context.reset();
}
