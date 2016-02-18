#include "processscheduler.h"
#include "process.h"
#include <algorithm>

namespace killme
{
    ProcessScheduler::ProcessScheduler()
        : processes_()
        , idCounter_(0)
    {
    }

    Process ProcessScheduler::startProcess(ProcessFun fun, size_t priority)
    {
        const auto point = std::lower_bound(std::cbegin(processes_), std::cend(processes_), priority,
            [](const std::pair<size_t, ProcessInfo>& a, size_t pri) { return a.second.priority < pri; });

        const auto id = idCounter_++;
        ProcessInfo info;
        info.priority = priority;
        info.fun = fun;

        processes_.emplace(point, id, info);

        return Process(shared_from_this(), id, priority);
    }

    void ProcessScheduler::killProcess(size_t id, size_t priority)
    {
        const auto begin = std::lower_bound(std::cbegin(processes_), std::cend(processes_), priority,
            [](const std::pair<size_t, ProcessInfo>& a, size_t pri) { return a.second.priority < pri; });

        const auto end = std::cend(processes_);
        const auto it = std::find_if(begin, end, [&](const std::pair<size_t, ProcessInfo>& a) { return a.first == id; });
        if (it != end)
        {
            processes_.erase(it);
        }
    }

    void ProcessScheduler::update(size_t begin, size_t end)
    {
        const auto b = std::cbegin(processes_);
        const auto e = std::cend(processes_);

        auto range = std::make_pair(
            std::lower_bound(b, e, begin, [](const std::pair<size_t, ProcessInfo>& a, size_t pri) { return a.second.priority < pri; }),
            std::upper_bound(b, e, end - 1, [](size_t pri, const std::pair<size_t, ProcessInfo>& a) { return a.second.priority > pri; })
            );

        while (range.first != range.second)
        {
            range.first->second.fun();
            ++range.first;
        }
    }
}