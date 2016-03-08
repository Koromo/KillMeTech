#include "eventdispatcher.h"
#include "../core/utility.h"
#include <unordered_map>

namespace killme
{
    namespace detail
    {
        struct DispatcherImpl
        {
            std::unordered_multimap<std::string, std::pair<size_t, EventDispatcher::EventHook>> hookMap_;
            UniqueCounter<size_t> uniqueId_;

        public:
            size_t connect(const std::string& type, EventDispatcher::EventHook hook)
            {
                const auto id = uniqueId_();
                hookMap_.emplace(type, std::make_pair(id, hook));
                return id;
            }

            void disconnect(const std::string& type, size_t id)
            {
                auto range = hookMap_.equal_range(type);
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

            void emit(const Event& e)
            {
                auto range = hookMap_.equal_range(toLowers(e.getType()));
                while (range.first != range.second)
                {
                    range.first->second.second(e);
                    ++range.first;
                }
            }
        };

        Disconnector::~Disconnector()
        {
            if (const auto d = dispatcher.lock())
            {
                d->disconnect(type, id);
            }
        }
    }

    EventConnection::EventConnection(const std::shared_ptr<detail::Disconnector>& disconnector)
        : disconnector_(disconnector)
    {
    }

    void EventConnection::disconnect()
    {
        disconnector_.reset();
    }

    EventDispatcher::EventDispatcher()
        : impl_(std::make_shared<detail::DispatcherImpl>())
    {
    }

    EventConnection EventDispatcher::connect(const std::string& type, EventHook hook)
    {
        const auto lowers = toLowers(type);
        const auto id = impl_->connect(lowers, hook);

        const auto disconnector = std::make_shared<detail::Disconnector>();
        disconnector->id = id;
        disconnector->type = lowers;
        disconnector->dispatcher = impl_;
        return EventConnection(disconnector);
    }

    void EventDispatcher::emit(const Event& e)
    {
        impl_->emit(e);
    }
}