#include "eventdispatcher.h"
#include "event.h"
#include "eventhook.h"
#include "../core/string.h"

namespace killme
{
    void EventDispatcher::addHook(const std::string& type, const std::shared_ptr<EventHook>& hook)
    {
        hookMap_.insert({toLowers(type), hook});
    }

    void EventDispatcher::removeHook(const std::string& type, const std::shared_ptr<EventHook>& hook)
    {
        auto range = hookMap_.equal_range(toLowers(type));
        while (range.first != range.second)
        {
            if (range.first->second == hook)
            {
                hookMap_.erase(range.first);
                return;
            }
            ++range.first;
        }
    }

    void EventDispatcher::dispatch(const Event& e)
    {
        auto range = hookMap_.equal_range(toLowers(e.getType()));
        while (range.first != range.second)
        {
            range.first->second->onEvent(e);
            ++range.first;
        }
    }
}