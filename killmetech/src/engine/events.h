#ifndef _KILLME_EVENTS_H_
#define _KILLME_EVENTS_H_

#include "../events/eventdispatcher.h"
#include <string>

namespace killme
{
    class EventConnection;
    class Event;

    /** Event subsystem interface */
    struct Events
    {
        /** Startup event subsystem */
        static void startup();

        /** Shutdown event subsystem */
        static void shutdown();

        /** Add event hook */
        static EventConnection connect(const std::string& type, EventDispatcher::EventHook hook);

        /** Triggr an event */
        static void emit(const Event& e);
    };
}

#endif