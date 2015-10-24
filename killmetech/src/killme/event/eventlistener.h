#ifndef _KILLME_EVENTLISTENER_H_
#define _KILLME_EVENTLISTENER_H_

namespace killme
{
    class Event;

    /** Event listener interface */
    class EventListener
    {
    public:
        /** For drived classes */
        virtual ~EventListener() = default;

        /** Process event */
        virtual void onEvent(const Event& e) = 0;
    };
}

#endif