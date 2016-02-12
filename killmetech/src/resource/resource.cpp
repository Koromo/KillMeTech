#include "resource.h"
#include "resourcemanager.h"

namespace killme
{
    std::shared_ptr<IsResource> detail::loadResource(const std::string& path)
    {
        return resourceManager.load(path);
    }

    void detail::unloadResource(const std::string& path)
    {
        return resourceManager.unload(path);
    }
}