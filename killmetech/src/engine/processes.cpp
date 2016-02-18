#include "processes.h"
#include "../processes/process.h"
#include <memory>

namespace killme
{
    const int PROCESS_PRIORITY_PHYSICS          = 100;
    const int PROCESS_PRIORITY_AUDIO_LISTENER   = 200;
    const int PROCESS_PRIORITY_AUDIO_EMITTER    = 300;
    const int PROCESS_PRIORITY_SCENE            = 400;
    const int PROCESS_PRIORITY_MAX              = 1000000;

    namespace
    {
        std::shared_ptr<ProcessScheduler> globalScheduler;
    }

    void Processes::startup()
    {
        globalScheduler = std::make_shared<ProcessScheduler>();
    }

    void Processes::shutdown()
    {
        globalScheduler.reset();
    }

    Process Processes::start(ProcessScheduler::ProcessFun fun, size_t priority)
    {
        return globalScheduler->startProcess(fun, priority);
    }

    void Processes::update(size_t begin, size_t end)
    {
        globalScheduler->update(begin, end);
    }
}