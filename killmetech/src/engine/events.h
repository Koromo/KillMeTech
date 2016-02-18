#ifndef _KILLME_EVENTS_H_
#define _KILLME_EVENTS_H_

#include "../events/eventdispatcher.h"
#include <string>

namespace killme
{
    class EventConnection;
    class Event;

    struct Events
    {
        static void startup();
        static void shutdown();
        static EventConnection connect(const std::string& type, EventDispatcher::EventHook hook);
        static void emit(const Event& e);
    };
}

#endif