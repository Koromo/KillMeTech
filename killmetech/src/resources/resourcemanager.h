#ifndef _KILLME_RESOURCEMANAGER_H_
#define _KILLME_RESOURCEMANAGER_H_

#include "resource.h"
#include "../core/string.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <cassert>

namespace killme
{
    /** The resource manager */
    class ResourceManager
    {
    public:
        using Loader = std::function<std::shared_ptr<IsResource>(const std::string&)>;

    private:
        std::unordered_map<std::string, Loader> loaderMap_;
        std::unordered_map<std::string, std::shared_ptr<IsResource>> resourceMap_;

    public:
        /** Sets the resource loader */
        void registerLoader(const std::string& ext, Loader loader);

        /** Removes the resource loader */
        void unregisterLoader(const std::string& ext);

        /** Returns resource accessor */
        template <class T>
        Resource<T> getAccessor(const std::string& path, bool immediateLoading)
        {
            const auto lowers = toLowers(path);
            const auto it = resourceMap_.find(lowers);
            if (it == std::cend(resourceMap_))
            {
                const Resource<T> accessor(*this, lowers, std::shared_ptr<T>());
                if (immediateLoading)
                {
                    accessor.load();
                }
                return accessor;
            }

            const auto resource = std::dynamic_pointer_cast<T>(it->second);
            assert(resource && "Mismatch the resource type.");

            return Resource<T>(*this, lowers, resource);
        }

        /** Loads a resource */
        std::shared_ptr<IsResource> load(const std::string& path);

        /** Unloads a resource */
        void unload(const std::string& path);
    };

    template <class T>
    Resource<T> accessResource(ResourceManager& manager, const std::string& path, bool immediateLoading = true)
    {
        return manager.template getAccessor<T>(path, immediateLoading);
    }

    template <class T, class Loader>
    Resource<T> accessResource(Loader loader, const std::shared_ptr<T>& resource = nullptr)
    {
        return Resource<T>(loader, resource);
    }
}

#endif