#include "resourcemanager.h"

namespace killme
{
    void ResourceManager::registerLoader(const std::string& ext, Loader loader)
    {
        const auto check = loaderMap_.emplace(toLowers(ext), loader);
        assert(check.second && "Conflicts the resource loader.");
    }

    void ResourceManager::unregisterLoader(const std::string& ext)
    {
        loaderMap_.erase(toLowers(ext));
    }

    namespace detail
    {
        std::string getExtension(const std::string& path)
        {
            const auto found = path.rfind('.');
            assert(found != std::string::npos && "File extension not exists.");
            return path.substr(found + 1);
        }
    }

    std::shared_ptr<IsResource> ResourceManager::load(const std::string& path)
    {
        const auto lowers = toLowers(path);
        const auto ext = detail::getExtension(lowers);
        const auto resource = loaderMap_.at(ext)(lowers);
        resourceMap_[lowers] = resource;
        return resource;
    }

    void ResourceManager::unload(const std::string& path)
    {
        resourceMap_.erase(toLowers(path));
    }
}