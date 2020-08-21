#include <dlfcn.h>
#include <cstdio>
#include <subhook.h>
#include <mutex>
#include <memory>

#include "repeater.hpp"

extern "C" void glClear(GLbitfield)
{
    puts("Yeah, I am a fake original glClear\n");
}


using namespace ve;

struct EnhancerContext
{
    SymbolRedirection redictions;
    Repeater repeater;
    subhook_t dlsymhook;
};

static std::unique_ptr<EnhancerContext> context = nullptr;

namespace ve
{
    
    void* hooked_dlsym(void* params, const char* symbol)
    {
        static std::mutex dlsym_mutex;
        auto lock = std::unique_lock<std::mutex>(dlsym_mutex);

        if(context->redictions.hasRedirection(symbol))
        {
            auto* const targetAddress = context->redictions.getTarget(symbol);
            return targetAddress;
        }
        /*
         * Prepare call to original dlsym code
         */
        auto original_dlsym = subhook_get_trampoline(context->dlsymhook);
        using dlsym_type = void*(void*, const char*);
        return reinterpret_cast<dlsym_type*>(original_dlsym)(params, symbol);
    }
}


/**
 * @brief Execute when DLL is loaded to process
 *
 * Hooks all neccessary functions to detour OpenGL library calls
 */
__attribute((constructor)) void setup()
{
    context = std::make_unique<EnhancerContext>();
    /*
     * Hook dlopen/dlsym functions and dispatch via our own function
     */
    context->dlsymhook = subhook_new(reinterpret_cast<void *>(dlsym), reinterpret_cast<void *>(hooked_dlsym), static_cast<subhook_flags>(0));
    auto retval = subhook_install(context->dlsymhook);
    if(retval != 0)
    {
        fputs("Enhancer failed: failed to hook dlsym()\n",stdout);
        return;
    }

    /*
     * Set original symbol getter using trampolined dlsym()
     */
    context->repeater.setSymbolGetter([](const char* symbol)->void* 
    {
        puts("Calling original dlsym with symbol\n");
        auto original_dlsym = subhook_get_trampoline(context->dlsymhook);
        if(original_dlsym == NULL)
        {
            puts("Trampoline for dlsym failed\n");
        }
        using dlsym_type = void*(void*, const char*);
        dlsym_type* original = reinterpret_cast<dlsym_type*>(original_dlsym);
        auto result = original(RTLD_DEFAULT, "glClear");
        puts("Returned from func\n");
        if(result == NULL)
        {
            puts("Failed to get original address via dlsym()");
        }
        return result;
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
