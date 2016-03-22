#ifndef _KILLME_COMMANDALLOCATOR_H_
#define _KILLME_COMMANDALLOCATOR_H_

#include "renderdevice.h"
#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** The CommandAllocator */
    class CommandAllocator : public RenderDeviceChild
    {
    private:
        ComUniquePtr<ID3D12CommandAllocator> allocator_;
        bool protected_;

    public:
        /** Initialize */
        void initialize();

        /** Reset commands memory */
        void reset();

        /** Protect for reset */
        void protect(bool b);

        /** If true, this allocator is not resetable */
        bool isProtected() const;

        /** Return Direct3D command allocator */
        ID3D12CommandAllocator* getD3DAllocator();
    };
}

#endif