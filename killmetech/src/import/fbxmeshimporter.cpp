#include "fbxmeshimporter.h"
#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexdata.h"
#include "../resource/resourcemanager.h"
#include "../core/exception.h"
#include <stack>
#include <vector>
#include <cstdlib>
#include <cassert>

namespace killme
{
    FbxMeshImporter::FbxMeshImporter()
        : fbxManager_(makeFbxUnique(FbxManager::Create()))
    {
        const auto ios = FbxIOSettings::Create(fbxManager_.get(), IOSROOT);
        fbxManager_->SetIOSettings(ios);
    }

    namespace
    {
        struct MeshCache
        {
            std::vector<float> controlPoints;
            std::vector<unsigned short> indices;
        };

        // Parse positions
        void storeControlPoints(const FbxMesh* mesh, MeshCache& out)
        {
            const auto numControlPoints = mesh->GetControlPointsCount();
            out.controlPoints.resize(numControlPoints * 3);

            const auto controlPoints = mesh->GetControlPoints();
            for (int i = 0; i < numControlPoints; ++i)
            {
                const auto cp = controlPoints[i];
                out.controlPoints[i * 3] = static_cast<float>(cp[0]);
                out.controlPoints[i * 3 + 1] = static_cast<float>(cp[1]);
                out.controlPoints[i * 3 + 2] = static_cast<float>(cp[2]);
            }
        }

        // Parse indices
        void storeIndices(const FbxMesh* mesh, MeshCache& out)
        {
            const auto numIndices = mesh->GetPolygonVertexCount();
            out.indices.resize(numIndices);

            const auto numPolygons = mesh->GetPolygonCount();
            for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
            {
                assert(mesh->GetPolygonSize(polygonIndex) == 3 && "Not supportted to the no triangle polygon.");
                out.indices[polygonIndex * 3] = mesh->GetPolygonVertex(polygonIndex, 0);
                out.indices[polygonIndex * 3 + 1] = mesh->GetPolygonVertex(polygonIndex, 1);
                out.indices[polygonIndex * 3 + 2] = mesh->GetPolygonVertex(polygonIndex, 2);
            }
        }

        // Parse mesh
        std::shared_ptr<Mesh> parseMeshScene(const FbxNode* node)
        {
            std::shared_ptr<Mesh> parsedMesh = std::make_shared<Mesh>();

            std::stack<const FbxNode*> stack;
            stack.push(node);

            while (!stack.empty())
            {
                const auto top = stack.top();
                stack.pop();

                const auto attribute = top->GetNodeAttribute();
                if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
                {
                    const auto fbxMesh = static_cast<const FbxMesh*>(attribute);

                    // Parse the fbx mesh and create mesh resources
                    MeshCache cache;
                    storeControlPoints(fbxMesh, cache);
                    storeIndices(fbxMesh, cache);

                    const auto positionBuffer = renderSystem.createVertexBuffer(cache.controlPoints.data(), sizeof(float) * cache.controlPoints.size(), sizeof(float) * 3);
                    const auto indexBuffer = renderSystem.createIndexBuffer(cache.indices.data(), sizeof(unsigned short) * cache.indices.size());

                    const auto vertexData = std::make_shared<VertexData>();
                    vertexData->addVertices(VertexSemantic::position, 0, positionBuffer);
                    vertexData->setIndices(indexBuffer);

                    const auto material = getManagedResource<Material>("media/box.material");

                    parsedMesh->createSubMesh(fbxMesh->GetName(), vertexData, material);
                }

                const auto numChildren = top->GetChildCount();
                for (int i = 0; i < numChildren; ++i)
                {
                    stack.push(top->GetChild(i));
                }
            }

            return parsedMesh;
        }
    }

    std::shared_ptr<Mesh> FbxMeshImporter::import(const std::string& path)
    {
        // Get fullpath
        char fullpash[512];
        _fullpath(fullpash, path.c_str(), 512);

        // Initialize importer
        const auto importer = makeFbxUnique(FbxImporter::Create(fbxManager_.get(), ""));
        enforce<FbxException>(
            importer->Initialize(fullpash, -1, fbxManager_->GetIOSettings()),
            "Failed to initialize Fbx importer (" + path + ").\n" +
            std::string(importer->GetStatus().GetErrorString()));

        // Import scene
        const auto scene = makeFbxUnique(FbxScene::Create(fbxManager_.get(), ""));
        enforce<FbxException>(
            importer->Import(scene.get()),
            "Failed to import Fbx scene (" + path + ").\n" +
            std::string(importer->GetStatus().GetErrorString()));

        // Fix axis to the Direct3D
        const auto axis = scene->GetGlobalSettings().GetAxisSystem();
        if (axis != FbxAxisSystem::DirectX)
        {
            FbxAxisSystem::DirectX.ConvertScene(scene.get());
        }

        // Convert meshes
        FbxGeometryConverter converter(fbxManager_.get());
        converter.Triangulate(scene.get(), true); // Triangle polygon
        converter.SplitMeshesPerMaterial(scene.get(), true); // Submeshes per materials

        return parseMeshScene(scene->GetRootNode());
    }
}