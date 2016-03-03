#include "eventdispatcher.h"
#include "event.h"
#include "../core/string.h"

namespace killme
{
    EventDispatcher eventDispatcher;

    detail::Disconnector::~Disconnector()
    {
        disconnect();
    }

    void detail::Disconnector::disconnect()
    {
        if (const auto o = owner.lock())
        {
            o->disconnect(type, id);
        }
    }

    EventConnection::EventConnection(const std::shared_ptr<detail::Disconnector>& disconnector)
        : disconnector_(disconnector)
    {
    }

    void EventConnection::disconnect()
    {
        if (disconnector_)
        {
            disconnector_.reset();
        }
    }

    EventDispatcher::EventDispatcher()
        : hookMap_()
        , idCounter_(0)
    {
    }

    EventConnection EventDispatcher::connect(const std::string& type, EventHook hook)
    {
        const auto lowers = toLowers(type);
        const auto id = idCounter_++;
        hookMap_.emplace(toLowers(type), std::make_pair(id, hook));

        const auto disconnector = std::make_shared<detail::Disconnector>();
        disconnector->id = id;
        disconnector->type = lowers;
        disconnector->owner = shared_from_this();

        return EventConnection(disconnector);
    }

    void EventDispatcher::disconnect(const std::string& type, size_t id)
    {
        auto range = hookMap_.equal_range(toLowers(type));
        while (range.first != range.second)
        {
            if (range.first->second.first == id)
            {
                hookMap_.erase(range.first);
                return;
            }
            ++range.first;
        }
    }

    void EventDispatcher::emit(const Event& e)
    {
        auto range = hookMap_.equal_range(toLowers(e.getType()));
        while (range.first != range.second)
        {
            range.first->second.second(e);
            ++range.first;
        }
    }
}