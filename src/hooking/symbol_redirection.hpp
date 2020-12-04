#include <unordered_map>
#include <string>
namespace ve
{
namespace hooking
{
    /**
     * @brief 
     */
    class SymbolRedirection
    {
        private:
            using dictType = std::unordered_map<std::string, void*>;
            dictType mapping;
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
        
            const dictType& getMapping() const
            {
                return mapping;
            }
    };
} //namespace hooking
} //namespace ve
