#include "resource.h"
#include "resourcemanager.h"

namespace killme
{
    std::shared_ptr<IsResource> detail::callLoad(ResourceManager& msg, const std::string& path)
    {
        return msg.load(path);
    }

    void detail::callUnload(ResourceManager& msg, const std::string& path)
    {
        return msg.unload(path);
    }
}