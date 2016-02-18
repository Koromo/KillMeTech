#include "resources.h"

namespace killme
{
    ResourceManager* Resources::detail::resourceManager;

    ResourceManager& Resources::getManager()
    {
        return *detail::resourceManager;
    }

    void Resources::startup()
    {
        detail::resourceManager = new ResourceManager();
    }

    void Resources::shutdown()
    {
        detail::resourceManager;
    }

    void Resources::registerLoader(const std::string& ext, ResourceManager::Loader loader)
    {
        detail::resourceManager->registerLoader(ext, loader);
    }

    void Resources::unregisterLoader(const std::string& ext)
    {
        detail::resourceManager->unregisterLoader(ext);
    }
}