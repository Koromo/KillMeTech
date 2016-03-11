#ifndef _KILLME_PROCESS_H_
#define _KILLME_PROCESS_H_

#include <memory>

namespace killme
{
    /** Process deleter */
    struct ProcessKiller
    {
        virtual ~ProcessKiller() = default;
    };

    /** Process handle */
    class Process
    {
    private:
        std::shared_ptr<ProcessKiller> killer_;

    public:
        /** Construct */
        Process() = default;

        explicit Process(const std::shared_ptr<ProcessKiller>& killer)
            : killer_(killer)
        {
        }

        /** Kill process */
        void kill()
        {
            killer_.reset();
        }

        /** Whether process is running */
        operator bool() const
        {
            return !!killer_;
        }
    };
}

#endif