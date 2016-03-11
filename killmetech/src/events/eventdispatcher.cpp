#include "eventdispatcher.h"
#include "../core/utility.h"
#include "../core/string.h"
#include <unordered_map>
#include <vector>
#include <tuple>

namespace killme
{
    namespace detail
    {
        struct DispatcherImpl
        {
            std::unordered_multimap<std::string, std::pair<size_t, EventDispatcher::EventHook>> hookMap_;
            std::vector<std::tuple<std::string, size_t, EventDispatcher::EventHook>> connects_;
            std::vector<std::tuple<std::string, size_t>> disconnects_;

            UniqueCounter<size_t> uniqueId_;

        public:
            size_t connect(const std::string& type, EventDispatcher::EventHook hook)
            {
                const auto id = uniqueId_();
                connects_.emplace_back(type, id, hook);
                return id;
            }

            void disconnect(const std::string& type, size_t id)
            {
                disconnects_.emplace_back(type, id);
            }

            void emit(const Event& e)
            {
                for (const auto& t : connects_)
                {
                    hookMap_.emplace(std::get<0>(t), std::make_pair(std::get<1>(t), std::get<2>(t)));
                }
                for (const auto& t : disconnects_)
                {
                    auto range = hookMap_.equal_range(std::get<0>(t));
                    while (range.first != range.second)
                    {
                        if (range.first->second.first == std::get<1>(t))
                        {
                            range.first = hookMap_.erase(range.first);
                        }
                        else
                        {
                            ++range.first;
                        }
                    }
                }

                connects_.clear();
                disconnects_.clear();

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