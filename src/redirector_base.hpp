#ifndef __REDIRECTOR_BASE_HPP
#define __REDIRECTOR_BASE_HPP
#include <functional>
#include "symbol_redirection.hpp"

/// Original symbol getter type
using original_symbol_getter = std::function<void*(const char* symbolName)>;

namespace ve
{
    class RedirectorBase
    {
        private:
            original_symbol_getter getter;
        protected:
            SymbolRedirection redirector;
        public:
        virtual ~RedirectorBase() = default;
        virtual void setSymbolGetter(original_symbol_getter getter)
        {
            this->getter = getter;
        }

        virtual void registerCallbacks() = 0;

        void* getOriginalSymbolAddress(const char* symbolName)
        {
            return getter(symbolName);
        }

        const SymbolRedirection& getRedirectedFunctions() const
        {
            return redirector;
        }
    };
} // namespace ve
#endif
