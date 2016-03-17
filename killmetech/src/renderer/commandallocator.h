#ifndef _KILLME_COMMANDALLOCATOR_H_
#define _KILLME_COMMANDALLOCATOR_H_

#include "d3dsupport.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <d3d12.h>
#include <cassert>

namespace killme
{
    /** The CommandAllocator */
    class CommandAllocator
    {
    private:
        ComUniquePtr<ID3D12CommandAllocator> allocator_;
        bool locked_;

    public:
        /** Construct */
        explicit CommandAllocator(ID3D12CommandAllocator* allocator)
            : allocator_(makeComUnique(allocator))
            , locked_(false) {}

        /** Reset commands memory */
        void reset()
        {
            assert(!locked_ && "This CommandAllocator is locking.");
            enforce<Direct3DException>(
                SUCCEEDED(allocator_->Reset()),
                "Failed to reset the CommandAllocator.");
        }

        /** Set locking state to this allocator */
        void lock(bool b) { locked_ = b; }

        /** Whether this allocator is locked or not */
        bool isLocked() const { return locked_; }

        /** Return Direct3D command allocator */
        ID3D12CommandAllocator* getD3DAllocator() { return allocator_.get(); }
    };
}

#endif