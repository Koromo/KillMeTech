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
    /** Resource manager */
    class ResourceManager
    {
    public:
        using Loader = std::function<std::shared_ptr<IsResource>(const std::string&)>;

    private:
        std::unordered_map<std::string, Loader> loaderMap_;
        std::unordered_map<std::string, std::shared_ptr<IsResource>> resourceMap_;

    public:
        /** Set the resource loader */
        void registerLoader(const std::string& ext, Loader loader);

        /** Remove the resource loader */
        void unregisterLoader(const std::string& ext);

        /** Access a resource */
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
            assert(resource && "Mismatch resource type.");

            return Resource<T>(*this, lowers, resource);
        }

        /** Load a resource */
        std::shared_ptr<IsResource> load(const std::string& path);

        /** Unload a resource */
        void unload(const std::string& path);
    };
}

#endif