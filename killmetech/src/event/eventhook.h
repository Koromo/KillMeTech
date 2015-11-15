#ifndef _KILLME_EVENTHOOK_H_
#define _KILLME_EVENTHOOK_H_

namespace killme
{
    class Event;

    /** Event hook interface */
    class EventHook
    {
    public:
        /** For drived classes */
        virtual ~EventHook() = default;

        /** Process event */
        virtual void onEvent(const Event& e) = 0;
    };
}

#endif