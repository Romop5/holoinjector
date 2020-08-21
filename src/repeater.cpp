#include "repeater.hpp"
#include <iostream>

using namespace ve;

void Repeater::registerCallbacks(SymbolRedirection& redirector) 
{
    registerOpenGLSymbols({"glClear"}, redirector);
}

void Repeater::glClear(GLbitfield mask)
{
    printf("[Repeater] Redirecting glClear\n");
    OpenglRedirectorBase::glClear(mask);
} 

