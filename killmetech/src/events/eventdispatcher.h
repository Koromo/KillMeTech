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

    /** The handle of event hook */
    class EventConnection
    {
    private:
        std::shared_ptr<detail::Disconnector> disconnector_;

    public:
        /** Constructs */
        EventConnection() = default;
        explicit EventConnection(const std::shared_ptr<detail::Disconnector>& disconnector);
        EventConnection(const EventConnection&) = default;
        EventConnection(EventConnection&&) = default;

        /** Assignment operator */
        EventConnection& operator =(const EventConnection&) = default;
        EventConnection& operator =(EventConnection&&) = default;

        /** Removes event hook from the dispather */
        void disconnect();
    };

    /** Dispatch event for listeners */
    class EventDispatcher : public std::enable_shared_from_this<EventDispatcher>
    {
    public:
        using EventHook = std::function<void(const Event&)>;

    private:
        std::unordered_multimap<std::string, std::pair<size_t, EventHook>> hookMap_;
        size_t idCounter_;

    public:
        /** Constructs */
        EventDispatcher();

        /** Adds an event hook */
        EventConnection connect(const std::string& type, EventHook hook);

        /** Removes an event hook */
        void disconnect(const std::string& type, size_t id);

        /** Dispatches the event */
        /// NOTE: When event is emitting, calling the connect() or disconnect() is not permitted
        void emit(const Event& e);
    };
}

#endif