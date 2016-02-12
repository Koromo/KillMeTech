#include "eventmanager.h"
#include "event.h"
#include "../core/string.h"

namespace killme
{
    EventManager eventManager;

    detail::Disconnector::Disconnector(const std::string& type, size_t id)
        : type_(type)
        , id_(id)
    {
    }

    detail::Disconnector::~Disconnector()
    {
        disconnect();
    }

    void detail::Disconnector::disconnect()
    {
        eventManager.disconnect(type_, id_);
    }

    EventConnection::EventConnection(const std::string& type, size_t id)
        : disconnector_(std::make_shared<detail::Disconnector>(type, id))
    {
    }

    void EventConnection::disconnect()
    {
        disconnector_->disconnect();
    }

    void EventManager::startup()
    {
        idCounter_ = 0;
    }

    void EventManager::shutdown()
    {
        hookMap_.clear();
    }

    EventConnection EventManager::connect(const std::string& type, EventHook hook)
    {
        const auto lowers = toLowers(type);
        const auto id = idCounter_++;

        const auto it = hookMap_.find(lowers);
        if (it == std::cend(hookMap_))
        {
            hookMap_[lowers].insert({ id, hook });
        }
        else
        {
            it->second.insert({ id, hook });
        }

        return EventConnection(type, id);
    }

    void EventManager::disconnect(const std::string& type, size_t id)
    {
        const auto it = hookMap_.find(toLowers(type));
        if (it != std::cend(hookMap_))
        {
            const auto it2 = it->second.find(id);
            if (it2 != std::cend(it->second))
            {
                it->second.erase(it2);
            }
        }
    }

    void EventManager::emit(const Event& e)
    {
        const auto it = hookMap_.find(toLowers(e.getType()));
        if (it != std::cend(hookMap_))
        {
            for (auto hook : it->second)
            {
                hook.second(e);
            }
        }
    }
}