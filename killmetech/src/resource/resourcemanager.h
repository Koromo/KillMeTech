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
    private:
        using Loader = std::function<std::shared_ptr<IsResource>(const std::string&)>;

        std::unordered_map<std::string, Loader> loaderMap_;
        std::unordered_map<std::string, std::shared_ptr<IsResource>> resourceMap_;

    public:
        /** Initializes */
        void startup();

        /** Finalizes */
        void shutdown();

        /** Sets the resource loader */
        void setLoader(const std::string& ext, Loader loader);

        /** Removes the resource loader */
        void removeLoader(const std::string& ext);

        // For getManagedResource()
        template <class T>
        Resource<T> getInterface(const std::string& path, bool immediateLoading)
        {
            const auto lowers = toLowers(path);
            const auto it = resourceMap_.find(lowers);
            if (it == std::cend(resourceMap_))
            {
                const Resource<T> inter(lowers, std::shared_ptr<T>());
                if (immediateLoading)
                {
                    inter.load();
                }
                return inter;
            }

            const auto resource = std::dynamic_pointer_cast<T>(it->second);
            assert(resource && "Mismatch the resource type.");

            return Resource<T>(lowers, resource);
        }

        /** Loads a resource */
        std::shared_ptr<IsResource> load(const std::string& path);

        /** Unloads a resource */
        void unload(const std::string& path);
    };

    extern ResourceManager resourceManager;

    /** Returns the interface of resource */
    template <class T>
    Resource<T> getManagedResource(const std::string& path, bool immediateLoading = true)
    {
        return resourceManager.template getInterface<T>(path, immediateLoading);
    }

    template <class T>
    Resource<T> getNonmanagedResource(std::function<std::shared_ptr<T>()> loader, bool immediateLoading = true)
    {
        if (immediateLoading)
        {
            return Resource<T>(loader(), loader);
        }
        return Resource<T>(nullptr, loader);
    }
}

#endif