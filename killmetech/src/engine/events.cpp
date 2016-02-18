#include "events.h"
#include <memory>

namespace killme
{
    namespace
    {
        std::shared_ptr<EventDispatcher> globalDispatcher;
    }

    void Events::startup()
    {
        globalDispatcher = std::make_shared<EventDispatcher>();
    }

    void Events::shutdown()
    {
        globalDispatcher.reset();
    }

    EventConnection Events::connect(const std::string& type, EventDispatcher::EventHook hook)
    {
        return globalDispatcher->connect(type, hook);
    }
    
    void Events::emit(const Event& e)
    {
        globalDispatcher->emit(e);
    }
}