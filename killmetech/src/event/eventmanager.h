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

    /** The handle of event hook */
    class EventHookHandle
    {
    private:
        std::string type_;
        size_t id_;

    public:
        /** Constructs */
        EventHookHandle(const std::string& type, size_t id);

        /** Calls EventHookHandle::disconnect() */
        ~EventHookHandle();

        /** Returns the event type */
        std::string getType() const;

        /** Returns the id */
        size_t getId() const;

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
        /** Constructs */
        EventManager();

        /** Adds an event hook */
        std::shared_ptr<EventHookHandle> connect(const std::string& type, EventHook hook);

        /** Removes an event hook */
        void disconnect(const std::shared_ptr<EventHookHandle>& handle);
        void disconnect(const std::string& type, size_t id);

        /** Removes all event hooks */
        void disconnectAll();

        /** Dispatches the event */
        void emit(const Event& e);
    };

    extern EventManager eventManager;
}

#endif