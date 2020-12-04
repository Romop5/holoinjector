#include <string_view>
#include <vector>

namespace ve
{
namespace pipeline
{
    std::vector<std::string_view> tokenize(const std::string_view& code);    

    /// Verifies if 'token' names a GLSL builtin type
    bool isBuiltinGLSLType(const std::string_view& token);
}; //namespace pipeline
}; //namespace ve
