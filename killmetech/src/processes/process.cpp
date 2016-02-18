#include "process.h"
#include "processscheduler.h"

namespace killme
{
    detail::Killer::Killer(const std::weak_ptr<ProcessScheduler>& scheduler, size_t id, size_t priority)
        : scheduler_(scheduler)
        , id_(id)
        , priority_(priority)
    {
    }

    detail::Killer::~Killer()
    {
        kill();
    }

    void detail::Killer::kill()
    {
        if (const auto s = scheduler_.lock())
        {
            s->killProcess(id_, priority_);
        }
    }

    Process::Process(const std::weak_ptr<ProcessScheduler>& scheduler, size_t id, size_t priority)
        : killer_(std::make_shared<detail::Killer>(scheduler, id, priority))
    {
    }

    void Process::kill()
    {
        killer_->kill();
    }
}