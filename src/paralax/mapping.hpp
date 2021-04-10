/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        paralax/mapping.hpp
*
*****************************************************************************/

#ifndef PARALAX_MAPPING_HPP
#define PARALAX_MAPPING_HPP

#include <memory>

namespace ve
{
namespace utils
{
    class glProgram;
    class glFullscreenVAO;
};
namespace paralax
{
    /**
     * @brief Produce N-paralax pictures from input framebuffer using Steep paralax mapping
     */
    class Mapping
    {
    public:
        void initializeResources();
        void bindInputDepthBuffer(size_t bufferID);
        void bindInputColorBuffer(size_t bufferID);
        void draw(size_t gridXSize, size_t gridYSize, float disparityRatio, float centerRatio);

    private:
        void setUniform1i(const std::string& name, size_t value);
        void setUniform1f(const std::string& name, float value);
        std::shared_ptr<ve::utils::glProgram> m_program;
        std::shared_ptr<ve::utils::glFullscreenVAO> m_VAO;
    };
} //namespace paralax
}; // namespace ve

#endif
