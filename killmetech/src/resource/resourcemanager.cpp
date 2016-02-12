#include "resourcemanager.h"

namespace killme
{
    ResourceManager resourceManager;

    void ResourceManager::startup()
    {
    }

    void ResourceManager::shutdown()
    {
        resourceMap_.clear();
        loaderMap_.clear();
    }

    void ResourceManager::setLoader(const std::string& ext, Loader loader)
    {
        const auto lowers = toLowers(ext);
        
        //const auto it = loaderMap_.find(low);
        //assert(it == std::cend(loaderMap_) && "Conflicts resource loaders.");

        loaderMap_[lowers] = loader;
    }

    namespace detail
    {
        std::string getExtension(const std::string& path)
        {
            const auto found = path.rfind('.');
            if (found == std::string::npos)
            {
                assert(false && "Filename extension not exists.");
                return "";
            }
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
        const auto it = resourceMap_.find(path);
        if (it != std::cend(resourceMap_))
        {
            resourceMap_.erase(it);
        }
    }
}