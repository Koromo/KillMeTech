#include "vertexshader.h"
#include "d3dsupport.h"
#include "../core/exception.h"
#include <vector>
#include <cassert>

namespace killme
{
    const std::string VertexShader::model = "vs_5_0";

    namespace
    {
        // Returns vertex format
        DXGI_FORMAT vertexFormat(const std::string& semanticName)
        {
            if (semanticName == "POSITION") { return DXGI_FORMAT_R32G32B32_FLOAT; }
            if (semanticName == "COLOR")    { return DXGI_FORMAT_R32G32B32A32_FLOAT; }
            assert(false && "Invalid vertex semantic name.");
            return DXGI_FORMAT_UNKNOWN; // For wanings
        }
    }

    VertexShader::VertexShader(ID3DBlob* byteCode)
        : BasicShader(byteCode)
        , reflectionHolder_()
        , inputElems_()
        , inputLayout_()
    {
        // Get shader reflection
        ID3D12ShaderReflection* reflection;
        enforce<Direct3DException>(SUCCEEDED(D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(&reflection))),
            "Failed to get shader reflection.");
        reflectionHolder_ = makeComUnique(reflection);

        // Get shader description
        D3D12_SHADER_DESC shaderDesc;
        enforce<Direct3DException>(SUCCEEDED(reflection->GetDesc(&shaderDesc)),
            "Failed to get shader description.");

        // Build input layout
        inputElems_.resize(shaderDesc.InputParameters);

        for (size_t i = 0; i < shaderDesc.InputParameters; ++i)
        {
            D3D12_SIGNATURE_PARAMETER_DESC param;
            enforce<Direct3DException>(SUCCEEDED(reflection->GetInputParameterDesc(i, &param)),
                "Failed to get shader input parameter description.");

            inputElems_[i].SemanticName = param.SemanticName;
            inputElems_[i].SemanticIndex = param.SemanticIndex;
            inputElems_[i].Format = vertexFormat(param.SemanticName);
            inputElems_[i].InputSlot = i;
            inputElems_[i].AlignedByteOffset = 0;
            inputElems_[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            inputElems_[i].InstanceDataStepRate = 0;
        }

        inputLayout_.pInputElementDescs = inputElems_.data();
        inputLayout_.NumElements = shaderDesc.InputParameters;
    }

    D3D12_INPUT_LAYOUT_DESC VertexShader::getD3DInputLayout() const
    {
        return inputLayout_;
    }
}