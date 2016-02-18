#ifndef _KILLME_PROCESSSCHEDULER_H_
#define _KILLME_PROCESSSCHEDULER_H_

#include <vector>
#include <functional>
#include <utility>
#include <memory>

namespace killme
{
    class Process;

    /** The processes scheduler */
    class ProcessScheduler : public std::enable_shared_from_this<ProcessScheduler>
    {
    public:
        using ProcessFun = std::function<void()>;

    private:
        struct ProcessInfo
        {
            ProcessFun fun;
            size_t priority;
        };

        std::vector<std::pair<size_t, ProcessInfo>> processes_;
        size_t idCounter_;

    public:
        /** Initializes */
        ProcessScheduler();

        /** Creates a process */
        Process startProcess(ProcessFun fun, size_t priority);

        /** Kills the process */
        void killProcess(size_t id, size_t priority);

        /** Updates processes that has priority (begin, end] */
        /// NOTE: When update processes, calling the startProcess() or killProcess() is not permitted
        void update(size_t begin, size_t end);
    };
}

#endif