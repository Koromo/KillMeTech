#include "resourcemanager.h"
#include "../core/string.h"
#include <cassert>

namespace killme
{
    void ResourceStore::registerLoader(const std::string& ext, ResourceLoader loader)
    {
        const auto check = loaderMap_.emplace(ext, loader);
        assert(check.second && ("Conflict the resource loader \'" + ext + "\'.").c_str());
    }

    void ResourceStore::unregisterLoader(const std::string& ext)
    {
        loaderMap_.erase(ext);
    }

    std::shared_ptr<IsResource> ResourceStore::getLoadedResource(const std::string& path)
    {
        const auto it = resourceMap_.find(toLowers(path));
        if (it == std::cend(resourceMap_))
        {
            return nullptr;
        }
        return it->second;
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

    std::shared_ptr<IsResource> ResourceStore::load(const std::string& path)
    {
        const auto lowers = toLowers(path);
        const auto ext = detail::getExtension(lowers);

        const auto loader = loaderMap_.find(ext);
        assert(loader != std::cend(loaderMap_) && ("A resource loader \'" + ext + "\' not exists.").c_str());

        const auto resource = loader->second(lowers);
        resourceMap_[lowers] = resource;
        return resource;
    }

    void ResourceStore::unload(const std::string& path)
    {
        resourceMap_.erase(toLowers(path));
    }

    ResourceManager::ResourceManager()
        : store_(std::make_shared<ResourceStore>())
    {
    }

    void ResourceManager::registerLoader(const std::string& ext, ResourceLoader loader)
    {
        store_->registerLoader(toLowers(ext), loader);
    }

    void ResourceManager::unregisterLoader(const std::string& ext)
    {
        store_->unregisterLoader(toLowers(ext));
    }

    std::weak_ptr<ResourceStore> ResourceManager::getStore()
    {
        return store_;
    }
}