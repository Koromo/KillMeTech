#ifndef _KILLME_PROCESS_H_
#define _KILLME_PROCESS_H_

#include <memory>

namespace killme
{
    class ProcessScheduler;

    namespace detail
    {
        struct Killer
        {
            std::weak_ptr<ProcessScheduler> scheduler_;
            size_t id_;
            size_t priority_;
            Killer(const std::weak_ptr<ProcessScheduler>& scheduler, size_t id, size_t priority);
            ~Killer();
            void kill();
        };
    }

    /** Process handle */
    class Process
    {
    private:
        std::shared_ptr<detail::Killer> killer_;

    public:
        /** Construct */
        Process() = default;
        Process(const std::weak_ptr<ProcessScheduler>& scheduler, size_t id, size_t priority);

        /** Kill process */
        void kill();
    };
}

#endif