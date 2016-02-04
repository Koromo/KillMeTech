#ifndef _KILLME_BOXMESH_H_
#define _KILLME_BOXMESH_H_

#include "mesh.h"
#include "material.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexdata.h"
#include "../renderer/rootsignature.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../core/string.h"
#include <memory>

namespace killme
{
    /** Util */
    inline std::shared_ptr<Mesh> makeBoxMesh(const std::shared_ptr<RenderSystem>& renderSystem)
    {
        const float positions[] = {
            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,

            -0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, 0.5f,

            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,

            0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,

            -0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f
        };
        const unsigned short indices[] = {
            3, 1, 0,
            2, 1, 3,
            6, 4, 5,
            7, 4, 6,
            11, 9, 8,
            10, 9, 11,
            14, 12, 13,
            15, 12, 14,
            19, 17, 16,
            18, 17, 19,
            22, 20, 21,
            23, 20, 22
        };

        const auto positionBuffer = renderSystem->createVertexBuffer(positions, sizeof(positions), sizeof(float) * 3);
        const auto indexBuffer = renderSystem->createIndexBuffer(indices, sizeof(indices));
        const auto vertexData = std::make_shared<VertexData>();
        vertexData->addVertices(VertexSemantic::position, 0, positionBuffer);
        vertexData->setIndices(indexBuffer);

        RootSignatureDescription rootSignatureDesc(1);
        rootSignatureDesc[0].set(0, 2, ShaderType::vertex);

        const auto rootSignature = renderSystem->createRootSignature(rootSignatureDesc);
        const auto vertexShader = compileShader<VertexShader>(KILLME_T("vs.hlsl"));
        const auto pixelShader = compileShader<PixelShader>(KILLME_T("ps.hlsl"));

        PipelineStateDescription pipelineStateDesc;
        pipelineStateDesc.rootSignature = rootSignature;
        pipelineStateDesc.vertexShader = vertexShader;
        pipelineStateDesc.pixelShader = pixelShader;
        const auto pipelineState = renderSystem->createPipelineState(pipelineStateDesc);

        const auto material = std::make_shared<Material>(pipelineState);

        return std::make_shared<Mesh>(vertexData, material);
    }
}

#endif