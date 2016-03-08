#ifndef _KILLME_RESOURCEMANAGER_H_
#define _KILLME_RESOURCEMANAGER_H_

#include <memory>
#include <unordered_map>
#include <functional>
#include <string>

namespace killme
{
    class IsResource;

    /** Media resource loader */
    using ResourceLoader = std::function<std::shared_ptr<IsResource>(const std::string&)>;

    /** Media resource store */
    class ResourceStore
    {
    private:
        std::unordered_map<std::string, ResourceLoader> loaderMap_;
        std::unordered_map<std::string, std::shared_ptr<IsResource>> resourceMap_;

    public:
        /** Set a media resource loader */
        void registerLoader(const std::string& ext, ResourceLoader loader);

        /** Remove a media resource loader */
        void unregisterLoader(const std::string& ext);

        /** Return a loaded resource */
        std::shared_ptr<IsResource> getLoadedResource(const std::string& path);

        /** Load a resource */
        std::shared_ptr<IsResource> load(const std::string& path);

        /** Unload a resource */
        void unload(const std::string& path);
    };

    /** Media resource manager */
    class ResourceManager
    {
    private:
        std::shared_ptr<ResourceStore> store_;

    public:
        /** Construct */
        ResourceManager();

        /** Set the resource loader */
        void registerLoader(const std::string& ext, ResourceLoader loader);

        /** Remove the resource loader */
        void unregisterLoader(const std::string& ext);

        /** Returns resource store */
        std::weak_ptr<ResourceStore> getStore();
    };
}

#endif