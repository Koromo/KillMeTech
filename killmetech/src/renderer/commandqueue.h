#ifndef _KILLME_COMMANDQUEUE_H_
#define _KILLME_COMMANDQUEUE_H_

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
    class CommandQueue
    {
    private:
        ComUniquePtr<ID3D12CommandQueue> queue_;
        ComUniquePtr<ID3D12Fence> fence_;
        std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)> fenceEvent_;
        UINT64 fenceValue_;
        std::unordered_set<std::shared_ptr<CommandAllocator>> executingAllocators_;
        std::unordered_set<std::shared_ptr<CommandList>> executingCommands_;

    public:
        /** Construct */
        CommandQueue(ID3D12CommandQueue* queue, ID3D12Fence* fence)
            : queue_(makeComUnique(queue))
            , fence_(makeComUnique(fence))
            , fenceEvent_(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS), CloseHandle)
            , fenceValue_(0)
            , executingAllocators_()
            , executingCommands_()
        {
            enforce<Direct3DException>(
                SUCCEEDED(fence_->Signal(0)),
                "Failed to initialize CommandQueue.");
        }

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

                allocator->lock(true);
                list->lock(true);
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
        bool isCompleted() const { return fence_->GetCompletedValue() == fenceValue_; }

        /** Wait for commands execution */
        void waitForCommands()
        {
            if (!isCompleted())
            {
                enforce<Direct3DException>(
                    SUCCEEDED(fence_->SetEventOnCompletion(fenceValue_, fenceEvent_.get())),
                    "Failed to set the signal event.");
                WaitForSingleObject(fenceEvent_.get(), INFINITE);
            }
            updateExecutionState();
        }

        /** Update execution state */
        void updateExecutionState()
        {
            if (!isCompleted())
            {
                return;
            }

            for (const auto& a : executingAllocators_)
            {
                a->lock(false);
            }
            for (const auto& c : executingCommands_)
            {
                c->lock(false);
            }
            executingAllocators_.clear();
            executingCommands_.clear();
        }
    };
}

#endif
