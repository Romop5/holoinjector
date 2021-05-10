#include "pipeline/shader_profile.hpp"
#include <cassert>
#include <fstream>
#include <sstream>
#include "yaml-cpp/yaml.h"
#include "logger.hpp"

using namespace hi::pipeline;

namespace
{

    YAML::Node serializeEntry(const ProfileEntry& entry)
    {
        YAML::Node profile;
        if (!entry.transformationMatrixName.empty())
            profile["transformationMatrixName"] = entry.transformationMatrixName;
        return profile;
    }

    ProfileEntry deserializeEntry(YAML::Node config)
    {
        ProfileEntry result;
        if (config["transformationMatrixName"])
        {
            result.transformationMatrixName = config["transformationMatrixName"].as<std::string>();
            hi::Logger::logDebug("[ShaderProfile] Found transformationMatrixName: ", result.transformationMatrixName);
        }
        return result;
    }
}

ShaderProfile::ShaderProfile(std::filesystem::path dir) :
    searchDir(dir)
{
}

bool ShaderProfile::hasProfile(size_t hashValue)
{
    if (hasProfileInCache(hashValue))
        return true;
    searchForProfile(hashValue);
    return hasProfileInCache(hashValue);
}

const ProfileEntry& ShaderProfile::getProfile(size_t hashValue)
{
    assert(hasProfileInCache(hashValue));
    return cache[hashValue];
}

bool ShaderProfile::saveProfile(size_t hashValue, const ProfileEntry& entry)
{
    auto filePath = getProfilePath(hashValue);
    std::ofstream fout(filePath);
    if (!fout.is_open())
    {
        Logger::logError("[ShaderProfile] Failed to open ", std::filesystem::absolute(filePath), " for writing");
        return false;
    }

    auto profile = serializeEntry(entry);
    fout << profile;
    return true;
}

bool ShaderProfile::hasProfileInCache(size_t hashValue)
{
    return (cache.count(hashValue) > 0);
}

void ShaderProfile::searchForProfile(size_t hashValue)
{
    static std::vector<std::string> keyDefaultValues = {
        { "transformationMatrixName"},
    };

    auto filePath = getProfilePath(hashValue);

    try {
        Logger::logDebug("[ShaderProfile] Searching for ", std::filesystem::absolute(filePath));
        YAML::Node config = YAML::LoadFile(filePath);
        auto result = deserializeEntry(config); 
        cache[hashValue] = result;
    } catch (YAML::BadFile& e)
    {
        Logger::logDebug("[ShaderProfile] Profile for ", std::to_string(hashValue)," not found");
    }
}

std::filesystem::path ShaderProfile::getProfilePath(size_t hashValue)
{
    std::filesystem::path fileName = std::to_string(hashValue);
    std::filesystem::path filePath = searchDir / fileName;
    return filePath;
}
