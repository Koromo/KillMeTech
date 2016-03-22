#include "commandqueue.h"

namespace killme
{
    void CommandQueue::initialize()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc;
        ZeroMemory(&queueDesc, sizeof(queueDesc));
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ID3D12CommandQueue* queue;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue))),
            "Failed create the command queue.");
        queue_ = makeComUnique(queue);

        ID3D12Fence* fence;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))),
            "Failed to create the fence.");
        fence_ = makeComUnique(fence);

        fenceEvent_ = decltype(fenceEvent_)(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS), Closer());
        fenceValue_ = 0;
    }

    bool CommandQueue::isCompleted() const
    {
        return fence_->GetCompletedValue() == fenceValue_;
    }

    void CommandQueue::waitForCommands()
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

    void CommandQueue::updateExecutionState()
    {
        if (!isCompleted())
        {
            return;
        }

        for (const auto& a : executingAllocators_)
        {
            a->protect(false);
        }
        for (const auto& c : executingCommands_)
        {
            c->protect(false);
        }
        executingAllocators_.clear();
        executingCommands_.clear();
    }

    ID3D12CommandQueue* CommandQueue::getD3DCommandQueue()
    {
        return queue_.get();
    }
}