#include <dlfcn.h>
#include <cstdio>
#include <subhook.h>
#include <mutex>
#include <memory>

#include "repeater.hpp"

/*extern "C" void glClear(GLbitfield)
{
    puts("Yeah, I am a fake original glClear\n");
}*/


using namespace ve;

struct EnhancerContext
{
    SymbolRedirection redictions;
    Repeater repeater;
    subhook_t dlsymhook;
    subhook_t glxGetProcAddressHook;
    subhook_t glxGetProcAddressHookARB;
};

static std::unique_ptr<EnhancerContext> context = nullptr;

namespace ve
{
    void* original_glXGetProcAddress(const GLubyte * procName)
    {
        auto original = subhook_get_trampoline(context->glxGetProcAddressHook);
        return reinterpret_cast<decltype(original_glXGetProcAddress)*>(original)(procName);
    }

    void* hooked_glXGetProcAddress(const GLubyte * procName)
    {
        static std::mutex mutex;
        auto lock = std::unique_lock<std::mutex>(mutex);

        std::string symbol;
        for(size_t i = 0; procName[i] != '\0'; i++)
        {
            symbol.push_back(procName[i]);
        }

        printf("[Enhancer glxGetProcAddress] %s\n", procName);

        if(context->redictions.hasRedirection(symbol))
        {
            auto* const targetAddress = context->redictions.getTarget(symbol);
            return targetAddress;
        }

        return original_glXGetProcAddress(procName);
    }


    void* original_dlsym(void* params, const char* symbol)
    {
        /*
         * Prepare call to original dlsym code
         */
        auto original_dlsym = subhook_get_trampoline(context->dlsymhook);
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

        printf("[Enhancer dlsym] %s\n", symbol);

        if(symbol == "glXGetProcAddress")
            return reinterpret_cast<void*>(&hooked_glXGetProcAddress);

        if(symbol == "glXGetProcAddressARB")
            return reinterpret_cast<void*>(&hooked_glXGetProcAddress);

        if(context->redictions.hasRedirection(symbol))
        {
            auto* const targetAddress = context->redictions.getTarget(symbol);
            return targetAddress;
        }
        // Else, return original address
        return original_dlsym(params, symbol);
    }

    }


/**
 * @brief Execute when DLL is loaded to process
 *
 * Hooks all neccessary functions to detour OpenGL library calls
 */
__attribute((constructor)) void setup()
{
    // Decide whether to use 64bit hook or not
    auto hookTypeFlags = static_cast<subhook_flags>(SUBHOOK_BITS == 32?0:SUBHOOK_64BIT_OFFSET);
    context = std::make_unique<EnhancerContext>();
    /*
     * Hook dlopen/dlsym functions and dispatch via our own function
     */
    context->dlsymhook = subhook_new(reinterpret_cast<void *>(dlsym), reinterpret_cast<void *>(hooked_dlsym), hookTypeFlags);
    auto retval = subhook_install(context->dlsymhook);
    if(retval != 0)
    {
        fputs("Enhancer failed: failed to hook dlsym()\n",stdout);
        return;
    }

    auto trampoline_test = subhook_get_trampoline(context->dlsymhook);
    if(trampoline_test == NULL)
    {
        fputs("Enhancer failed: failed to create trampoline()\n",stdout);
        return;
    }

    void* glxGetProcAddressOriginal = original_dlsym(RTLD_NEXT, "glXGetProcAddress");
    if(glxGetProcAddressOriginal != nullptr)
    {
        /*
         * Hook dlopen/dlsym functions and dispatch via our own function
         */
        context->glxGetProcAddressHook = subhook_new(reinterpret_cast<void *>(glxGetProcAddressOriginal), reinterpret_cast<void *>(hooked_glXGetProcAddress), hookTypeFlags);
        retval = subhook_install(context->glxGetProcAddressHook);
        if(retval != 0)
        {
            fputs("Enhancer failed: failed to hook dlsym()\n",stdout);
            return;
        }
    }
    void* glxGetProcAddressARBOriginal = original_dlsym(RTLD_DEFAULT, "glXGetProcAddressARB");
    if(glxGetProcAddressARBOriginal != nullptr)
    {
        /*
         * Hook dlopen/dlsym functions and dispatch via our own function
         */
        context->glxGetProcAddressHookARB = subhook_new(reinterpret_cast<void *>(glxGetProcAddressARBOriginal), reinterpret_cast<void *>(hooked_glXGetProcAddress), static_cast<subhook_flags>(0));
        retval = subhook_install(context->glxGetProcAddressHookARB);
        if(retval != 0)
        {
            fputs("Enhancer failed: failed to hook dlsym()\n",stdout);
            return;
        }
    }

    /*
     * Set original symbol getter using trampolined dlsym()
     */
    context->repeater.setSymbolGetter([](const char* symbol)->void* 
    {
        printf("[Enhancer- symbol getter] Calling original dlsym with symbo %s\n",symbol);
        auto addr = ve::original_dlsym(RTLD_DEFAULT,symbol);
        if(addr == NULL)
        {
            puts("[Enhancer- symbol getter] Failed to get original address via dlsym()");
        }
        return addr;
    });    

    /*
     * Register OpenGL calls that should be redirected
     */
    context->repeater.registerCallbacks(context->redictions);    
    fputs("Registration done\n",stdout);

    // Test
    auto original_dlsym = subhook_get_trampoline(context->dlsymhook);
    using dlsym_type = void*(void*, const char*);
    dlsym_type* original = reinterpret_cast<dlsym_type*>(original_dlsym);
    auto originalAddr = original(RTLD_DEFAULT, "dlsym");
    puts("Original address\n");

}
