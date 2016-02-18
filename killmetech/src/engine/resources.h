#ifndef _KILLME_RESOURCES_H_
#define _KILLME_RESOURCES_H_

#include "../resources/resource.h"
#include "../resources/resourcemanager.h"

namespace killme
{
    namespace Resources
    {
        namespace detail
        {
            extern ResourceManager* resourceManager;
        }

        ResourceManager& getManager();

        void startup();
        void shutdown();

        void registerLoader(const std::string& ext, ResourceManager::Loader loader);
        void unregisterLoader(const std::string& ext);

        template <class T>
        Resource<T> load(const std::string& path)
        {
            return accessResource<T>(*detail::resourceManager, path);
        }
    }
}

#endif
