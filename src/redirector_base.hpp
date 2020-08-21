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
        public:
        virtual void setSymbolGetter(original_symbol_getter getter)
        {
            this->getter = getter;
        }

        virtual void registerCallbacks(SymbolRedirection& redirector) = 0;

        void* getOriginalSymbolAddress(const char* symbolName)
        {
            return getter(symbolName);
        }
    };
} // namespace ve
