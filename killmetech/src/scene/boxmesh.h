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
#include "../resource/resourcemanager.h"
#include <memory>

namespace killme
{
    /** Util */
    inline std::shared_ptr<Mesh> makeBoxMesh()
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

        const auto vertexShader = getResourceInterface<VertexShader>("vs.vhlsl");
        const auto pixelShader = getResourceInterface<PixelShader>("ps.phlsl");

        const auto mesh = std::make_shared<Mesh>();
        const std::string names[] = { "top", "bottom", "left", "right", "front", "back" };
        for (int i = 0; i < 6; ++i)
        {
            unsigned short localIndices[6];
            for (int j = 0; j < 6; ++j)
            {
                localIndices[j] = indices[i * 6 + j] - i * 4;
            }

            const auto positionBuffer = renderSystem.createVertexBuffer(positions + i * 12, sizeof(float) * 12, sizeof(float) * 3);
            const auto indexBuffer = renderSystem.createIndexBuffer(localIndices, sizeof(localIndices));
            const auto vertexData = std::make_shared<VertexData>();
            vertexData->addVertices(VertexSemantic::position, 0, positionBuffer);
            vertexData->setIndices(indexBuffer);

            const auto material = std::make_shared<Material>(vertexShader, pixelShader);
            mesh->createSubMesh(names[i], vertexData, material);
        }

        return mesh;
    }
}

#endif