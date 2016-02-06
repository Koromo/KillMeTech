#include "vertexshader.h"
#include "inputlayout.h"
#include "d3dsupport.h"
#include "../core/exception.h"
#include <vector>
#include <cassert>

namespace killme
{
    const std::string VertexShader::model = "vs_5_0";

    namespace
    {
        // Returns corresponded vertex format to vertex semantic
        DXGI_FORMAT vertexFormat(const std::string& semanticName)
        {
            if (semanticName == "POSITION") { return DXGI_FORMAT_R32G32B32_FLOAT; }
            if (semanticName == "COLOR")    { return DXGI_FORMAT_R32G32B32A32_FLOAT; }
            assert(false && "An invalid vertex semantic name.");
            return DXGI_FORMAT_UNKNOWN; // For warnings
        }
    }

    VertexShader::VertexShader(ID3DBlob* byteCode)
        : BasicShader(byteCode)
        , inputLayout_()
    {
        // Collect input elements
        std::vector<D3D12_INPUT_ELEMENT_DESC> elems;
        const auto signature = getD3DInputSignature();
        for (const auto& param: signature)
        {
            D3D12_INPUT_ELEMENT_DESC elem;
            elem.SemanticName = param.SemanticName;
            elem.SemanticIndex = param.SemanticIndex;
            elem.Format = vertexFormat(param.SemanticName);
            elem.InputSlot = elems.size();
            elem.AlignedByteOffset = 0;
            elem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elem.InstanceDataStepRate = 0;

            elems.push_back(elem);
        }

        inputLayout_ = std::make_shared<InputLayout>(std::move(elems));
    }

    std::shared_ptr<InputLayout> VertexShader::getInputLayout() const
    {
        return inputLayout_;
    }
}