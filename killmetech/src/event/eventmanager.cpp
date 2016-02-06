#include "eventmanager.h"
#include "event.h"
#include "../core/string.h"

namespace killme
{
    EventManager eventManager;

    EventHookHandle::EventHookHandle(const std::string& type, size_t id)
        : type_(type)
        , id_(id)
    {
    }

    EventHookHandle::~EventHookHandle()
    {
        disconnect();
    }

    std::string EventHookHandle::getType() const
    {
        return type_;
    }

    size_t EventHookHandle::getId() const
    {
        return id_;
    }

    void EventHookHandle::disconnect()
    {
        eventManager.disconnect(type_, id_);
    }

    EventManager::EventManager()
        : hookMap_()
        , idCounter_(0)
    {
    }

    std::shared_ptr<EventHookHandle> EventManager::connect(const std::string& type, EventHook hook)
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

        return std::make_shared<EventHookHandle>(type, id);
    }

    void EventManager::disconnect(const std::shared_ptr<EventHookHandle>& handle)
    {
        disconnect(handle->getType(), handle->getId());
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

    void EventManager::disconnectAll()
    {
        hookMap_.clear();
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