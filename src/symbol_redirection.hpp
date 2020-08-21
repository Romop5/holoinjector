#include <unordered_map>
#include <string>
namespace ve
{
    /**
     * @brief 
     */
    class SymbolRedirection
    {
        private:
            std::unordered_map<std::string, void*> mapping;
        public:
            SymbolRedirection() = default;
            bool hasRedirection(const std::string& symbolName) const
            {
                return mapping.count(symbolName) > 0;
            }
            void* getTarget(const std::string& symbolName) const
            {
                return mapping.at(symbolName);
            }

            void addRedirection(const std::string& symbol, void* address)
            {
                mapping[symbol] = address;
            }
    };
} //namespace ve
