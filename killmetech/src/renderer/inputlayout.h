#ifndef _KILLME_INPUTLAYOUT_H_
#define _KILLME_INPUTLAYOUT_H_

#include <d3d12.h>
#include <d3dcompiler.h>
#include <vector>
#include <utility>

namespace killme
{
    /** Input layout */
    class InputLayout
    {
    private:
        std::vector<D3D12_INPUT_ELEMENT_DESC> elems_;
        D3D12_INPUT_LAYOUT_DESC layout_;

    public:
        /** Constructs with Direct3D input element descriptions */
        template <class C>
        explicit InputLayout(C&& elems)
            : elems_(std::move(elems))
            , layout_()
        {
            layout_.pInputElementDescs = elems_.data();
            layout_.NumElements = elems_.size();
        }

        /** Returns a Direct3D Input layout */
        D3D12_INPUT_LAYOUT_DESC getD3DLayout() { return layout_; }
    };
}

#endif