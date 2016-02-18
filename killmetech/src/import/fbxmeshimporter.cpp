#include "fbxmeshimporter.h"
#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexdata.h"
#include "../resources/resourcemanager.h"
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
            std::vector<float> uvs;
            std::vector<float> normals;
            std::vector<float> colors;
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

        // Parse vertex texcoords
        void storeUVs(const FbxMesh* mesh, MeshCache& out)
        {
            const auto numUVElems = mesh->GetElementUVCount();
            assert(numUVElems <= 1 && "Not supported multiple geometry elements about texcoords.");

            if (numUVElems == 0)
            {
                return;
            }

            const auto numStrideElems = 2;
            const auto numControlPoints = mesh->GetControlPointsCount();
            out.uvs.resize(numControlPoints * numStrideElems);

            const auto uvElem = mesh->GetElementUV(0);
            const auto& directArray = uvElem->GetDirectArray();
            const auto& indexArray = uvElem->GetIndexArray();
            const auto mappingMode = uvElem->GetMappingMode();
            const auto referenceMode = uvElem->GetReferenceMode();

            switch (mappingMode)
            {
            case FbxGeometryElement::eByControlPoint:
                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = cpIndex;
                        out.uvs[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                        out.uvs[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eIndexToDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = indexArray.GetAt(cpIndex);
                        out.uvs[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                        out.uvs[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                    }
                    break;
                }

                default:
                    assert(false && "The not suppoted reference mode.");
                    break;
                }
                break;

            case FbxGeometryElement::eByPolygonVertex:
                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        const auto polygonSize = mesh->GetPolygonSize(polygonIndex);
                        assert(polygonSize == 3 && "Not supportted to no triangle polygon.");

                        for (int vertexId = 0; vertexId < polygonSize; ++vertexId)
                        {
                            const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexId);
                            const auto id = polygonIndex * 3 + vertexId;
                            out.uvs[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                            out.uvs[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                        }
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eIndexToDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        const auto polygonSize = mesh->GetPolygonSize(polygonIndex);
                        assert(polygonSize == 3 && "Not supportted to no triangle polygon.");

                        for (int vertexId = 0; vertexId < polygonSize; ++vertexId)
                        {
                            const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexId);
                            const auto id = indexArray.GetAt(polygonIndex * 3 + vertexId);
                            out.uvs[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                            out.uvs[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                        }
                    }
                    break;
                }

                default:
                    assert(false && "The not suppoted reference mode.");
                    break;
                }
                break;

            default:
                assert(false && "The not suppoted mapping mode.");
                break;
            }
        }

        // Parse vertex normals
        void storeNormals(const FbxMesh* mesh, MeshCache& out)
        {
            const auto numNormalElems = mesh->GetElementNormalCount();
            assert(numNormalElems <= 1 && "Not supported multiple geometry elements about normal.");

            if (numNormalElems == 0)
            {
                return;
            }

            const auto numStrideElems = 3;
            const auto numControlPoints = mesh->GetControlPointsCount();
            out.normals.resize(numControlPoints * numStrideElems);

            const auto normalElem = mesh->GetElementNormal(0);
            const auto& directArray = normalElem->GetDirectArray();
            const auto& indexArray = normalElem->GetIndexArray();
            const auto mappingMode = normalElem->GetMappingMode();
            const auto referenceMode = normalElem->GetReferenceMode();

            switch (mappingMode)
            {
            case FbxGeometryElement::eByControlPoint:
                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = cpIndex;
                        out.normals[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                        out.normals[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                        out.normals[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id)[2]);
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eIndexToDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = indexArray.GetAt(cpIndex);
                        out.normals[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                        out.normals[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                        out.normals[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id)[2]);
                    }
                    break;
                }

                default:
                    assert(false && "The not suppoted reference mode.");
                    break;
                }
                break;

            case FbxGeometryElement::eByPolygonVertex:
                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        const auto polygonSize = mesh->GetPolygonSize(polygonIndex);
                        assert(polygonSize == 3 && "Not supportted to no triangle polygon.");

                        for (int vertexId = 0; vertexId < polygonSize; ++vertexId)
                        {
                            const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexId);
                            const auto id = polygonIndex * 3 + vertexId;
                            out.normals[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                            out.normals[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                            out.normals[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id)[2]);
                        }
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eIndexToDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        const auto polygonSize = mesh->GetPolygonSize(polygonIndex);
                        assert(polygonSize == 3 && "Not supportted to no triangle polygon.");

                        for (int vertexId = 0; vertexId < polygonSize; ++vertexId)
                        {
                            const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexId);
                            const auto id = indexArray.GetAt(polygonIndex * 3 + vertexId);
                            out.normals[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id)[0]);
                            out.normals[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id)[1]);
                            out.normals[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id)[2]);
                        }
                    }
                    break;
                }

                default:
                    assert(false && "The not suppoted reference mode.");
                    break;
                }
                break;

            default:
                assert(false && "The not suppoted mapping mode.");
                break;
            }
        }

        // Parse vertex colors
        void storeColors(const FbxMesh* mesh, MeshCache& out)
        {
            const auto numColorElems = mesh->GetElementVertexColorCount();
            assert(numColorElems <= 1 && "Not supported multiple geometry elements about color.");

            if (numColorElems == 0)
            {
                return;
            }

            const auto numStrideElems = 4;
            const auto numControlPoints = mesh->GetControlPointsCount();
            out.colors.resize(numControlPoints * numStrideElems);

            const auto colorElem = mesh->GetElementVertexColor(0);
            const auto& directArray = colorElem->GetDirectArray();
            const auto& indexArray = colorElem->GetIndexArray();
            const auto mappingMode = colorElem->GetMappingMode();
            const auto referenceMode = colorElem->GetReferenceMode();

            switch (mappingMode)
            {
            case FbxGeometryElement::eByControlPoint:
                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = cpIndex;
                        out.colors[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id).mRed);
                        out.colors[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id).mGreen);
                        out.colors[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id).mBlue);
                        out.colors[cpIndex * numStrideElems + 3] = static_cast<float>(directArray.GetAt(id).mAlpha);
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eIndexToDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = indexArray.GetAt(cpIndex);
                        out.colors[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id).mRed);
                        out.colors[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id).mGreen);
                        out.colors[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id).mBlue);
                        out.colors[cpIndex * numStrideElems + 3] = static_cast<float>(directArray.GetAt(id).mAlpha);
                    }
                    break;
                }

                default:
                    assert(false && "The not suppoted reference mode.");
                    break;
                }
                break;

            case FbxGeometryElement::eByPolygonVertex:
                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        const auto polygonSize = mesh->GetPolygonSize(polygonIndex);
                        assert(polygonSize == 3 && "Not supportted to no triangle polygon.");

                        for (int vertexId = 0; vertexId < polygonSize; ++vertexId)
                        {
                            const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexId);
                            const auto id = polygonIndex * 3 + vertexId;
                            out.colors[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id).mRed);
                            out.colors[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id).mGreen);
                            out.colors[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id).mBlue);
                            out.colors[cpIndex * numStrideElems + 3] = static_cast<float>(directArray.GetAt(id).mAlpha);
                        }
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eIndexToDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        const auto polygonSize = mesh->GetPolygonSize(polygonIndex);
                        assert(polygonSize == 3 && "Not supportted to no triangle polygon.");

                        for (int vertexId = 0; vertexId < polygonSize; ++vertexId)
                        {
                            const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexId);
                            const auto id = indexArray.GetAt(polygonIndex * 3 + vertexId);
                            out.colors[cpIndex * numStrideElems] = static_cast<float>(directArray.GetAt(id).mRed);
                            out.colors[cpIndex * numStrideElems + 1] = static_cast<float>(directArray.GetAt(id).mGreen);
                            out.colors[cpIndex * numStrideElems + 2] = static_cast<float>(directArray.GetAt(id).mBlue);
                            out.colors[cpIndex * numStrideElems + 3] = static_cast<float>(directArray.GetAt(id).mAlpha);
                        }
                    }
                    break;
                }

                default:
                    assert(false && "The not suppoted reference mode.");
                    break;
                }
                break;

            default:
                assert(false && "The not suppoted mapping mode.");
                break;
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
        std::shared_ptr<Mesh> parseMeshScene(RenderSystem& renderSystem, ResourceManager& resourceManager, const FbxNode* node)
        {
            std::shared_ptr<Mesh> parsedMesh = std::make_shared<Mesh>();

            std::stack<const FbxNode*> stack;
            stack.emplace(node);

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
                    storeUVs(fbxMesh, cache);
                    storeNormals(fbxMesh, cache);
                    storeColors(fbxMesh, cache);
                    storeIndices(fbxMesh, cache);

                    const auto positionBuffer = renderSystem.createVertexBuffer(cache.controlPoints.data(), sizeof(float) * cache.controlPoints.size(), sizeof(float) * 3);
                    const auto indexBuffer = renderSystem.createIndexBuffer(cache.indices.data(), sizeof(unsigned short) * cache.indices.size());

                    const auto vertexData = std::make_shared<VertexData>();
                    vertexData->addVertices(SemanticNames::position, 0, positionBuffer);
                    vertexData->setIndices(indexBuffer);

                    if (!cache.uvs.empty())
                    {
                        const auto texcoordBuffer = renderSystem.createVertexBuffer(cache.uvs.data(), sizeof(float) * cache.uvs.size(), sizeof(float) * 2);
                        vertexData->addVertices(SemanticNames::texcoord, 0, texcoordBuffer);
                    }
                    if (!cache.normals.empty())
                    {
                        const auto normalBuffer = renderSystem.createVertexBuffer(cache.normals.data(), sizeof(float) * cache.normals.size(), sizeof(float) * 3);
                        vertexData->addVertices(SemanticNames::normal, 0, normalBuffer);
                    }
                    if (!cache.colors.empty())
                    {
                        const auto colorBuffer = renderSystem.createVertexBuffer(cache.colors.data(), sizeof(float) * cache.colors.size(), sizeof(float) * 4);
                        vertexData->addVertices(SemanticNames::color, 0, colorBuffer);
                    }

                    const auto material = accessResource<Material>(resourceManager, "media/box.material");
                    parsedMesh->createSubMesh(fbxMesh->GetName(), vertexData, material);
                }

                const auto numChildren = top->GetChildCount();
                for (int i = 0; i < numChildren; ++i)
                {
                    stack.emplace(top->GetChild(i));
                }
            }

            return parsedMesh;
        }
    }

    std::shared_ptr<Mesh> FbxMeshImporter::import(RenderSystem& renderSystem, ResourceManager& resourceManager, const std::string& path)
    {
        // Get fullpath
        /// TODO: _fullpath() is windows only
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

        return parseMeshScene(renderSystem, resourceManager, scene->GetRootNode());
    }
}