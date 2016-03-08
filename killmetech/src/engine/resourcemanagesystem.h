#ifndef _KILLME_RESOURCEMANAGESYSTEM_H_
#define _KILLME_RESOURCEMANAGESYSTEM_H_

#include "../resources/resource.h"
#include "../resources/resourcemanager.h"
#include <string>
#include <memory>

namespace killme
{
    class FbxMeshImporter;

    /** Media resource manager subsystem */
    class ResourceManageSystem
    {
    private:
        std::unique_ptr<ResourceManager> manager_;
        std::unique_ptr<FbxMeshImporter> fbxImporter_;

    public:
        // For loadResource()
        ResourceManager& getManager();

        /** Initialize resource manager */
        void startup();

        /** Finalize resource manager */
        void shutdown();

        /** Resource loader register */
        void registerLoader(const std::string& ext, ResourceLoader loader);
        void unregisterLoader(const std::string& ext);
    };

    extern ResourceManageSystem resourceManager;

    /** Return media resource accessor */
    template <class T>
    Resource<T> loadResource(const std::string& path)
    {
        const Resource<T> r(resourceManager.getManager(), path);
        r.access();
        return r;
    }
}

#endif
