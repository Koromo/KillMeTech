#ifndef _KILLME_CONSTANTBUFFER_H_
#define _KILLME_CONSTANTBUFFER_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /// TODO: GPU is read only?
    /// TODO: Type UINT or size_t
    /** Constant buffer */
    class ConstantBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        size_t bufferSize_;
        void* mappedData_;
        size_t dataSize_;

    public:
        /** Construct with Direct3D buffer */
        ConstantBuffer(ID3D12Resource* buffer, size_t bufferSize, size_t dataSize);

        /** Update buffer data */
        void update(const void* src);

        /** Returns GPU buffer location */
        D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress();

        /** Returns buffer size */
        size_t getBufferSize() const;
    };
}

#endif
