#ifndef _KILLME_RENDERQUEUE_H_
#define _KILLME_RENDERQUEUE_H_

#include <memory>
#include <queue>

namespace killme
{
    struct SceneContext;

    /** Entity renderer */
    class Renderer
    {
    public:
        /** For drived classes */
        virtual ~Renderer() = default;

        /** Executes rendering */
        virtual void recordCommands(const SceneContext& context) = 0;
    };

    /** The render queue */
    class RenderQueue
    {
    private:
        std::queue<std::shared_ptr<Renderer>> queue_;

    public:
        /** Construsts */
        RenderQueue()
            : queue_()
        {
        }

        /** Whether queue is empty */
        bool empty() const
        {
            return queue_.empty();
        }

        /** Pushs */
        void push(const std::shared_ptr<Renderer>& r)
        {
            queue_.push(r);
        }

        /** Pops */
        std::shared_ptr<Renderer> pop()
        {
            const auto r = queue_.front();
            queue_.pop();
            return r;
        }
    };
}

#endif