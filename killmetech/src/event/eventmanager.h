#ifndef _KILLME_EVENTMANAGER_H_
#define _KILLME_EVENTMANAGER_H_

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <utility>

namespace killme
{
    class Event;

    namespace detail
    {
        struct Disconnector
        {
            std::string type_;
            size_t id_;

            Disconnector(const std::string& type, size_t id);
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
        EventConnection(const std::string& type, size_t id);

        /** Removes event hook from the dispather */
        void disconnect();
    };

    /** Dispatch event for listeners */
    class EventManager
    {
    private:
        using EventHook = std::function<void(const Event&)>;

        std::unordered_map<std::string, std::unordered_map<size_t, EventHook>> hookMap_;
        size_t idCounter_;

    public:
        /** Initializes */
        void startup();

        /** Finalizes */
        void shutdown();

        /** Adds an event hook */
        EventConnection connect(const std::string& type, EventHook hook);

        /** Removes an event hook */
        void disconnect(const std::string& type, size_t id);

        /** Dispatches the event */
        void emit(const Event& e);
    };

    extern EventManager eventManager;
}

#endif