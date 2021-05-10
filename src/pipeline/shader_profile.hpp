/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/shader_profile.hpp
*
*****************************************************************************/

#ifndef HI_SHADER_PROFILE_HPP
#define HI_SHADER_PROFILE_HPP

#include <map>
#include <filesystem>

namespace hi
{
namespace pipeline
{
    struct ProfileEntry
    {
        std::string transformationMatrixName;
    };
    /*
     * @brief Provides user-defined settings for given shader
     */
    class ShaderProfile 
    {
    public:
        ShaderProfile() = default;

        explicit ShaderProfile(std::filesystem::path dir);

        bool hasProfile(size_t hashValue);
        const ProfileEntry& getProfile(size_t hashValue);
        bool saveProfile(size_t hashValue, const ProfileEntry& entry);
    protected:
        bool hasProfileInCache(size_t hashValue);
        void searchForProfile(size_t hashValue);
        std::filesystem::path getProfilePath(size_t hashValue);
    private:
        /// A cache of loaded profiles. Each profile is inserted in upon loading.
        std::map<size_t, ProfileEntry> cache;

        /// The directory to use when searching for profiles
        std::filesystem::path searchDir;
    };
} //namespace pipeline
} //namespace hi
#endif
