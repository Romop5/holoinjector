#include "repeater.hpp"
#include <iostream>

using namespace ve;

void Repeater::registerCallbacks(SymbolRedirection& redirector) 
{
    registerOpenGLSymbols({"glClear"}, redirector);
}

void Repeater::glClear(GLbitfield mask)
{
    std::cout << "Redirecting glClear()" << std::endl;
    OpenglRedirectorBase::glClear(mask);
} 

