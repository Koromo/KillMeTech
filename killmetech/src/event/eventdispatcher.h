#ifndef _KILLME_EVENTDISPATCHER_H_
#define _KILLME_EVENTDISPATCHER_H_

#include <unordered_map>
#include <string>
#include <memory>

namespace killme
{
    class Event;
    class EventHook;

    /** Dispatch event for listeners */
    class EventDispatcher
    {
    private:
        std::unordered_multimap<std::string, std::shared_ptr<EventHook>> hookMap_;

    public:
        /** Add event hook */
        void addHook(const std::string& type, const std::shared_ptr<EventHook>& hook);

        /** Remove event hook */
        void removeHook(const std::string& type, const std::shared_ptr<EventHook>& hook);

        /** Dispatch event */
        void dispatch(const Event& e);
    };
}

#endif