#include "eventdispatcher.h"
#include "event.h"
#include "eventlistener.h"
#include "../core/string.h"

namespace killme
{
    void EventDispatcher::addListener(const std::string& type, const std::shared_ptr<EventListener>& listener)
    {
        listenerMap_.insert({toLowers(type), listener});
    }

    void EventDispatcher::removeListener(const std::string& type, const std::shared_ptr<EventListener>& listener)
    {
        auto range = listenerMap_.equal_range(toLowers(type));
        while (range.first != range.second)
        {
            if (range.first->second == listener)
            {
                listenerMap_.erase(range.first);
                return;
            }
            ++range.first;
        }
    }

    void EventDispatcher::dispatch(const Event& e)
    {
        auto range = listenerMap_.equal_range(toLowers(e.getType()));
        while (range.first != range.second)
        {
            range.first->second->onEvent(e);
            ++range.first;
        }
    }
}