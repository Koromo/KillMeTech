#ifndef _KILLME_PROCESSES_H_
#define _KILLME_PROCESSES_H_

#include "../processes/processscheduler.h"

namespace killme
{
    class Process;

    extern const int PROCESS_PRIORITY_PHYSICS;          /// 100
    extern const int PROCESS_PRIORITY_AUDIO_LISTENER;   /// 200
    extern const int PROCESS_PRIORITY_AUDIO_EMITTER;    /// 300 
    extern const int PROCESS_PRIORITY_SCENE;            /// 400
    extern const int PROCESS_PRIORITY_MAX;              /// 1,000,000

    struct Processes
    {
        static void startup();
        static void shutdown();
        static Process start(ProcessScheduler::ProcessFun fun, size_t priority);
        static void update(size_t begin, size_t end);
    };
}

#endif