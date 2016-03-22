#include "commandallocator.h"
#include "d3dsupport.h"
#include "../core/exception.h"
#include <cassert>

namespace killme
{
    void CommandAllocator::initialize()
    {
        ID3D12CommandAllocator* allocator;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))),
            "Failed to create the CommandAllocator.");
        allocator_ = makeComUnique(allocator);
        protected_ = false;
    }

    void CommandAllocator::reset()
    {
        assert(!protected_ && "This CommandAllocator is protected.");
        enforce<Direct3DException>(
            SUCCEEDED(allocator_->Reset()),
            "Failed to reset the CommandAllocator.");
    }

    void CommandAllocator::protect(bool b)
    {
        protected_ = b;
    }

    bool CommandAllocator::isProtected() const
    {
        return protected_;
    }

    ID3D12CommandAllocator* CommandAllocator::getD3DAllocator()
    {
        return allocator_.get();
    }
}