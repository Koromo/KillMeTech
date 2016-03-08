#ifndef _KILLME_EVENTDISPATCHER_H_
#define _KILLME_EVENTDISPATCHER_H_

#include "event.h"
#include <string>
#include <memory>
#include <functional>
#include <utility>

namespace killme
{
    namespace detail
    {
        struct DispatcherImpl;

        struct Disconnector
        {
            std::weak_ptr<DispatcherImpl> dispatcher;
            std::string type;
            size_t id;
            ~Disconnector();
        };
    }

    /** Handler of an event hook */
    class EventConnection
    {
    private:
        std::shared_ptr<detail::Disconnector> disconnector_;

    public:
        /** Construct */
        EventConnection() = default;

        explicit EventConnection(const std::shared_ptr<detail::Disconnector>& disconnector);

        EventConnection(const EventConnection&) = default;
        EventConnection(EventConnection&&) = default;

        /** Assignment operator */
        EventConnection& operator =(const EventConnection&) = default;
        EventConnection& operator =(EventConnection&&) = default;

        /** Remove event hook from the owner dispather */
        void disconnect();
    };

    /** Dispatch events for listeners */
    class EventDispatcher
    {
    public:
        using EventHook = std::function<void(const Event&)>;

    private:
        std::shared_ptr<detail::DispatcherImpl> impl_;

    public:
        /** Construct */
        EventDispatcher();

        /** For drived classes */
        virtual ~EventDispatcher() = default;

        /** Add an event hook */
        EventConnection connect(const std::string& type, EventHook hook);

        /** Dispatch an event */
        /// NOTE: When event is emitting, calling the connect() or disconnect() is not permitted
        void emit(const Event& e);

        /** ditto */
        template <class... Params>
        void emit(const std::string& type, Params&&... params)
        {
            emit(Event(type, std::forward<Params>(params)...));
        }
    };
}

#endif