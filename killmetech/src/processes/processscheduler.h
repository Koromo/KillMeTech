#ifndef _KILLME_PROCESSSCHEDULER_H_
#define _KILLME_PROCESSSCHEDULER_H_

#include "process.h"
#include "../core/utility.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <utility>
#include <memory>

namespace killme
{
    /** Process function */
    template <class... Args>
    using ProcessFun = std::function<void(Args...)>;

    /** Process store */
    template <class... Args>
    class ProcessStore
    {
    private:
        std::unordered_map<size_t, ProcessFun<Args...>> processes_;
        std::vector<std::pair<size_t, ProcessFun<Args...>>> starts_;
        std::vector<size_t> kills_;

        UniqueCounter<size_t> id_;

    public:
        size_t startProcess(ProcessFun<Args...> fun)
        {
            const auto id = id_();
            starts_.emplace_back(id, fun);
            return id;
        }

        void killProcess(size_t id)
        {
            kills_.emplace_back(id);
        }

        template <class... Types>
        void update(Types&&... args)
        {
            for (const auto& start : starts_)
            {
                processes_.emplace(start.first, start.second);
            }
            for (const auto& kill : kills_)
            {
                processes_.erase(kill);
            }

            starts_.clear();
            kills_.clear();

            for (const auto& p : processes_)
            {
                p.second(std::forward<Types>(args)...);
            }
        }
    };

    namespace detail
    {
        template <class... Args>
        struct Killer : ProcessKiller
        {
            std::weak_ptr<ProcessStore<Args...>> store;
            size_t id;

            ~Killer()
            {
                if (const auto s = store.lock())
                {
                    s->killProcess(id);
                }
            }
        };
    }

    /** Processes scheduler */
    template <class... Args>
    class ProcessScheduler
    {
    private:
        std::shared_ptr<ProcessStore<Args...>> store_;

    public:
        /** Construct */
        ProcessScheduler()
            : store_(std::make_shared<ProcessStore<Args...>>())
        {
        }

        /** Create a process */
        Process startProcess(ProcessFun<Args...> fun)
        {
            const auto id = store_->startProcess(fun);
            const auto killer = std::make_shared<detail::Killer<Args...>>();
            killer->store = store_;
            killer->id = id;
            return Process(killer);
        }

        /** Update processes */
        template <class... Types>
        void update(Types&&... args)
        {
            store_->update(std::forward<Types>(args)...);
        }
    };
}

#endif