/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/shader_parser.hpp
*
*****************************************************************************/

#include <string_view>
#include <vector>

namespace hi
{
namespace pipeline
{
    std::vector<std::string_view> tokenize(const std::string_view& code);

    /// Verifies if 'token' names a GLSL builtin type
    bool isBuiltinGLSLType(const std::string_view& token);
}; //namespace pipeline
}; //namespace hi
