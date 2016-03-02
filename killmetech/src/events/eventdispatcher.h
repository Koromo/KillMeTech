#ifndef _KILLME_EVENTDISPATCHER_H_
#define _KILLME_EVENTDISPATCHER_H_

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <utility>

namespace killme
{
    class Event;
    class EventDispatcher;

    namespace detail
    {
        struct Disconnector
        {
            std::weak_ptr<EventDispatcher> owner;
            std::string type;
            size_t id;

            ~Disconnector();
            void disconnect();
        };
    }

    /** Handler of event hook */
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

        /** Remove event hook from the dispather */
        void disconnect();
    };

    /** Dispatch events for listeners */
    class EventDispatcher : public std::enable_shared_from_this<EventDispatcher>
    {
    public:
        using EventHook = std::function<void(const Event&)>;

    private:
        std::unordered_multimap<std::string, std::pair<size_t, EventHook>> hookMap_;
        size_t idCounter_;

    public:
        /** Construct */
        EventDispatcher();

        /** Add an event hook */
        EventConnection connect(const std::string& type, EventHook hook);

        /** Remove an event hook */
        void disconnect(const std::string& type, size_t id);

        /** Dispatche an event */
        /// NOTE: When event is emitting, calling the connect() or disconnect() is not permitted
        void emit(const Event& e);
    };
}

#endif