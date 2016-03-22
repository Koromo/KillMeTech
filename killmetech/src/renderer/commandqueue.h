#ifndef _KILLME_COMMANDQUEUE_H_
#define _KILLME_COMMANDQUEUE_H_

#include "renderdevice.h"
#include "commandallocator.h"
#include "commandlist.h"
#include "d3dsupport.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <Windows.h>
#include <d3d12.h>
#include <vector>
#include <unordered_set>

namespace killme
{
    /** The CommandQueue */
    class CommandQueue : public RenderDeviceChild
    {
    private:
        struct Closer
        {
            void operator()(HANDLE h)
            {
                CloseHandle(h);
            }
        };

        ComUniquePtr<ID3D12CommandQueue> queue_;
        ComUniquePtr<ID3D12Fence> fence_;
        std::unique_ptr<std::remove_pointer_t<HANDLE>, Closer> fenceEvent_;
        UINT64 fenceValue_;
        std::unordered_set<std::shared_ptr<CommandAllocator>> executingAllocators_;
        std::unordered_set<std::shared_ptr<CommandList>> executingCommands_;

    public:
        /** Initialize */
        void initialize();

        /** Execute commands */
        /// NOTE: When this queue is processing any commands yet, wait for complete before execute commands.
        template <class Range>
        void executeCommands(const Range& commands)
        {
            //waitForCommands();

            std::vector<ID3D12CommandList*> d3dCommands;
            for (const auto& list : commands)
            {
                const auto allocator = list->getAllocator();

                allocator->protect(true);
                list->protect(true);
                executingAllocators_.emplace(allocator);
                executingCommands_.emplace(list);

                d3dCommands.emplace_back(list->getD3DCommandList());
            }

            queue_->ExecuteCommandLists(d3dCommands.size(), d3dCommands.data());

            ++fenceValue_;
            enforce<Direct3DException>(
                SUCCEEDED(queue_->Signal(fence_.get(), fenceValue_)),
                "Failed to signal of command queue.");
            waitForCommands(); /// TODO: We does not want to wait
        }

        /** Whether commands execution is finished or not */
        bool isCompleted() const;

        /** Wait for commands execution */
        void waitForCommands();

        /** Update execution state */
        void updateExecutionState();

        /** Return Direct3D command queue */
        ID3D12CommandQueue* getD3DCommandQueue();
    };
}

#endif
