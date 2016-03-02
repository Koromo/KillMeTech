#ifndef _KILLME_RESOURCES_H_
#define _KILLME_RESOURCES_H_

#include "../resources/resource.h"
#include "../resources/resourcemanager.h"

namespace killme
{
    /** Resource manager subsystem */
    struct Resources
    {
        struct detail
        {
            static ResourceManager* resourceManager;
        };

        /** Return resource manager */
        static ResourceManager& getManager();

        /** Initialize */
        static void startup();

        /** Finalize */
        static void shutdown();

        /** Resource loader register */
        static void registerLoader(const std::string& ext, ResourceManager::Loader loader);
        static void unregisterLoader(const std::string& ext);

        /** Return resource accessor */
        template <class T>
        static Resource<T> load(const std::string& path)
        {
            return detail::resourceManager->getAccessor<T>(path, true);
        }
    };
}

#endif
