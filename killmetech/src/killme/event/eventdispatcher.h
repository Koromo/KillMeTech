#ifndef _KILLME_EVENTDISPATCHER_H_
#define _KILLME_EVENTDISPATCHER_H_

#include <unordered_map>
#include <string>
#include <memory>

namespace killme
{
    class Event;
    class EventListener;

    /** Dispatch event for listeners */
    class EventDispatcher
    {
    private:
        std::unordered_multimap<std::string, std::shared_ptr<EventListener>> listenerMap_;

    public:
        /** Add event listener */
        void addListener(const std::string& type, const std::shared_ptr<EventListener>& listener);

        /** Remove event listener */
        void removeListener(const std::string& type, const std::shared_ptr<EventListener>& listener);

        /** Dispatch event */
        void dispatch(const Event& e);
    };
}

#endif