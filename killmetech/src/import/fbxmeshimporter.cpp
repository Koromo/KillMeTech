#include "fbxmeshimporter.h"
#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../scene/materialcreation.h"
#include "../renderer/renderdevice.h"
#include "../renderer/commandlist.h"
#include "../renderer/commandqueue.h"
#include "../renderer/gpuresource.h"
#include "../renderer/vertexdata.h"
#include "../resources/resource.h"
#include "../core/exception.h"
#include <stack>
#include <vector>
#include <utility>
#include <cstdlib>

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
            std::vector<float> positions;
            std::vector<float> uvs;
            std::vector<float> normals;
            std::vector<float> colors;
            std::vector<unsigned short> indices;
        };

        const size_t POLYGON_SIZE = 3; // Triange polygon

        // Convert vertices mapped by control point to mapped by polygon vertex
        std::vector<float> toByPolygonVertexFromByControlPoint(const FbxMesh* mesh, const std::vector<float>& byCP, size_t numElemsInStride)
        {
            const auto numPolygonVertices = enforce<FbxException>(mesh->GetPolygonVertexCount(), "Not supportted .fbx format.");

            std::vector<float> byPV(numPolygonVertices * numElemsInStride);

            const auto numPolygons = mesh->GetPolygonCount();
            for (int polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
            {
                enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                {
                    const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                    const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexIndex);
                    for (size_t elemIndexInStride = 0; elemIndexInStride < numElemsInStride; ++elemIndexInStride)
                    {
                        byPV[pvIndex  * numElemsInStride + elemIndexInStride] = byCP[cpIndex * numElemsInStride + elemIndexInStride];
                    }
                }
            }

            return byPV;
        }

        // Parse positions
        void storePositions(const FbxMesh* mesh, MeshCache& out)
        {
            const auto NUM_ELEMS_IN_STRIDE = 3;

            const auto numControlPoints = mesh->GetControlPointsCount();
            std::vector<float> byCP(numControlPoints * NUM_ELEMS_IN_STRIDE);

            const auto controlPoints = mesh->GetControlPoints();
            for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
            {
                const auto cp = controlPoints[cpIndex];
                byCP[cpIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(cp[0]);
                byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(cp[1]);
                byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(cp[2]);
            }

            out.positions = toByPolygonVertexFromByControlPoint(mesh, byCP, NUM_ELEMS_IN_STRIDE);
        }

        // Parse indices
        /// TODO:
        void storeIndices(const FbxMesh* mesh, MeshCache& out)
        {
            const auto numPolygonVertices = enforce<FbxException>(mesh->GetPolygonVertexCount(), "Not supportted .fbx format.");

            out.indices.resize(numPolygonVertices);

            const auto numPolygons = mesh->GetPolygonCount();
            for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
            {
                enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                {
                    const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                    out.indices[pvIndex] = static_cast<unsigned char>(pvIndex);
                }
            }
        }

        // Parse vertex texcoords
        void storeUVs(const FbxMesh* mesh, MeshCache& out)
        {
            const auto NUM_ELEMS_IN_STRIDE = 2;

            const auto numUVElems = mesh->GetElementUVCount();
            enforce<FbxException>(numUVElems <= 1, "Not supportted .fbx format.");

            if (numUVElems == 0)
            {
                return;
            }

            const auto uvElem = mesh->GetElementUV(0);
            const auto& directArray = uvElem->GetDirectArray();
            const auto& indexArray = uvElem->GetIndexArray();
            const auto mappingMode = uvElem->GetMappingMode();
            const auto referenceMode = uvElem->GetReferenceMode();

            switch (mappingMode)
            {
            case FbxGeometryElement::eByControlPoint: {
                const auto numControlPoints = mesh->GetControlPointsCount();
                std::vector<float> byCP(numControlPoints * NUM_ELEMS_IN_STRIDE);

                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = cpIndex;
                        const auto uv = directArray.GetAt(id);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(uv[0]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(uv[1]);
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eIndexToDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = indexArray.GetAt(cpIndex);
                        const auto uv = directArray.GetAt(id);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(uv[0]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(uv[1]);
                    }
                    break;
                }

                default:
                    throw FbxException("Not supportted .fbx format.");
                }

                out.uvs = toByPolygonVertexFromByControlPoint(mesh, byCP, NUM_ELEMS_IN_STRIDE);
                break;
            }

            case FbxGeometryElement::eByPolygonVertex: {
                const auto numPolygonVertices = mesh->GetPolygonVertexCount();
                std::vector<float> byPV(numPolygonVertices * NUM_ELEMS_IN_STRIDE);

                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                        for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                        {
                            const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                            const auto id = pvIndex;
                            const auto uv = directArray.GetAt(id);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(uv[0]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(uv[1]);
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
                        enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                        for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                        {
                            const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                            const auto id = indexArray.GetAt(pvIndex);
                            const auto uv = directArray.GetAt(id);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(uv[0]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(uv[1]);
                        }
                    }
                    break;
                }

                default:
                    throw FbxException("Not supportted .fbx format.");
                }

                out.uvs = std::move(byPV);
                break;
            }

            default:
                throw FbxException("Not supportted .fbx format.");
            }
        }

        // Parse vertex normals
        void storeNormals(const FbxMesh* mesh, MeshCache& out)
        {
            const auto NUM_ELEMS_IN_STRIDE = 3;

            const auto numNormalElems = mesh->GetElementNormalCount();
            enforce<FbxException>(numNormalElems <= 1, "Not supportted .fbx format.");

            if (numNormalElems == 0)
            {
                return;
            }

            const auto normalElem = mesh->GetElementNormal(0);
            const auto& directArray = normalElem->GetDirectArray();
            const auto& indexArray = normalElem->GetIndexArray();
            const auto mappingMode = normalElem->GetMappingMode();
            const auto referenceMode = normalElem->GetReferenceMode();

            switch (mappingMode)
            {
            case FbxGeometryElement::eByControlPoint: {
                const auto numControlPoints = mesh->GetControlPointsCount();
                std::vector<float> byCP(numControlPoints * NUM_ELEMS_IN_STRIDE);

                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = cpIndex;
                        const auto normal = directArray.GetAt(id);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(normal[0]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(normal[1]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(normal[2]);
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eIndexToDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = indexArray.GetAt(cpIndex);
                        const auto normal = directArray.GetAt(id);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(normal[0]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(normal[1]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(normal[2]);
                    }
                    break;
                }

                default:
                    throw FbxException("Not supportted .fbx format.");
                }

                out.normals = toByPolygonVertexFromByControlPoint(mesh, byCP, NUM_ELEMS_IN_STRIDE);
                break;
            }

            case FbxGeometryElement::eByPolygonVertex: {
                const auto numPolygonVertices = mesh->GetPolygonVertexCount();
                std::vector<float> byPV(numPolygonVertices * NUM_ELEMS_IN_STRIDE);

                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                        for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                        {
                            const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                            const auto id = pvIndex;
                            const auto normal = directArray.GetAt(id);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(normal[0]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(normal[1]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(normal[2]);
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
                        enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                        for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                        {
                            const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                            const auto id = indexArray.GetAt(pvIndex);
                            const auto normal = directArray.GetAt(id);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(normal[0]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(normal[1]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(normal[2]);
                        }
                    }
                    break;
                }

                default:
                    throw FbxException("Not supportted .fbx format.");
                }

                out.normals = std::move(byPV);
                break;
            }

            default:
                throw FbxException("Not supportted .fbx format.");
            }
        }

        // Parse vertex colors
        void storeColors(const FbxMesh* mesh, MeshCache& out)
        {
            const auto NUM_ELEMS_IN_STRIDE = 4;

            const auto numColorElems = mesh->GetElementVertexColorCount();
            enforce<FbxException>(numColorElems <= 1, "Not supportted .fbx format.");

            if (numColorElems == 0)
            {
                return;
            }

            const auto colorElem = mesh->GetElementVertexColor(0);
            const auto& directArray = colorElem->GetDirectArray();
            const auto& indexArray = colorElem->GetIndexArray();
            const auto mappingMode = colorElem->GetMappingMode();
            const auto referenceMode = colorElem->GetReferenceMode();

            switch (mappingMode)
            {
            case FbxGeometryElement::eByControlPoint: {
                const auto numControlPoints = mesh->GetControlPointsCount();
                std::vector<float> byCP(numControlPoints * NUM_ELEMS_IN_STRIDE);

                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = cpIndex;
                        const auto color = directArray.GetAt(id);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(color[0]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(color[1]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(color[2]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 3] = static_cast<float>(color[3]);
                    }
                    break;
                }

                case FbxGeometryElement::eIndexToDirect: {
                    // MAPPING:     eByControlPoint
                    // REFERENCE:   eIndexToDirect
                    for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
                    {
                        const auto id = indexArray.GetAt(cpIndex);
                        const auto color = directArray.GetAt(id);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(color[0]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(color[1]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(color[2]);
                        byCP[cpIndex * NUM_ELEMS_IN_STRIDE + 3] = static_cast<float>(color[3]);
                    }
                    break;
                }

                default:
                    throw FbxException("Not supportted .fbx format.");
                }

                out.colors = toByPolygonVertexFromByControlPoint(mesh, byCP, NUM_ELEMS_IN_STRIDE);
                break;
            }

            case FbxGeometryElement::eByPolygonVertex: {
                const auto numPolygonVertices = mesh->GetPolygonVertexCount();
                std::vector<float> byPV(numPolygonVertices * NUM_ELEMS_IN_STRIDE);

                switch (referenceMode)
                {
                case FbxGeometryElement::eDirect: {
                    // MAPPING:     eByPolygonVertex
                    // REFERENCE:   eDirect
                    const auto numPolygons = mesh->GetPolygonCount();
                    for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
                    {
                        enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                        for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                        {
                            const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                            const auto id = pvIndex;
                            const auto color = directArray.GetAt(id);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(color[0]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(color[1]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(color[2]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 3] = static_cast<float>(color[3]);
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
                        enforce<FbxException>(mesh->GetPolygonSize(polygonIndex) == POLYGON_SIZE, "Not supportted .fbx format.");
                        for (int vertexIndex = 0; vertexIndex < POLYGON_SIZE; ++vertexIndex)
                        {
                            const auto pvIndex = polygonIndex * POLYGON_SIZE + vertexIndex;
                            const auto id = indexArray.GetAt(pvIndex);
                            const auto color = directArray.GetAt(id);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE] = static_cast<float>(color[0]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 1] = static_cast<float>(color[1]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 2] = static_cast<float>(color[2]);
                            byPV[pvIndex * NUM_ELEMS_IN_STRIDE + 3] = static_cast<float>(color[3]);
                        }
                    }
                    break;
                }

                default:
                    throw FbxException("Not supportted .fbx format.");
                }

                out.colors = std::move(byPV);
                break;
            }

            default:
                throw FbxException("Not supportted .fbx format.");
            }
        }

        // Parse mesh
        std::shared_ptr<Mesh> parseMeshScene(RenderDevice& device, ResourceManager& resources, const FbxNode* node)
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
                    storePositions(fbxMesh, cache);
                    storeUVs(fbxMesh, cache);
                    storeNormals(fbxMesh, cache);
                    storeColors(fbxMesh, cache);
                    storeIndices(fbxMesh, cache);

                    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
                    const auto vertexData = std::make_shared<VertexData>();
                    const auto allocator = device.obtainCommandAllocator();
                    const auto commands = device.obtainCommandList(allocator, nullptr);

                    const auto positionBuffer = device.createVertexBuffer(sizeof(float) * cache.positions.size(), sizeof(float) * 3, GpuResourceState::copyDestination);
                    commands->updateGpuResource(positionBuffer, cache.positions.data());
                    vertexBuffers.emplace_back(positionBuffer);
                    vertexData->addVertices(SemanticNames::position, 0, positionBuffer);

                    const auto indexBuffer = device.createIndexBuffer(sizeof(unsigned short) * cache.indices.size(), GpuResourceState::copyDestination);
                    commands->updateGpuResource(indexBuffer, cache.indices.data());
                    vertexData->setIndices(indexBuffer);

                    if (!cache.uvs.empty())
                    {
                        const auto texcoordBuffer = device.createVertexBuffer(sizeof(float) * cache.uvs.size(), sizeof(float) * 2, GpuResourceState::copyDestination);
                        commands->updateGpuResource(texcoordBuffer, cache.uvs.data());
                        vertexBuffers.emplace_back(texcoordBuffer);
                        vertexData->addVertices(SemanticNames::texcoord, 0, texcoordBuffer);
                    }
                    if (!cache.normals.empty())
                    {
                        const auto normalBuffer = device.createVertexBuffer(sizeof(float) * cache.normals.size(), sizeof(float) * 3, GpuResourceState::copyDestination);
                        commands->updateGpuResource(normalBuffer, cache.normals.data());
                        vertexBuffers.emplace_back(normalBuffer);
                        vertexData->addVertices(SemanticNames::normal, 0, normalBuffer);
                    }
                    if (!cache.colors.empty())
                    {
                        const auto colorBuffer = device.createVertexBuffer(sizeof(float) * cache.colors.size(), sizeof(float) * 4, GpuResourceState::copyDestination);
                        commands->updateGpuResource(colorBuffer, cache.colors.data());
                        vertexBuffers.emplace_back(colorBuffer);
                        vertexData->addVertices(SemanticNames::color, 0, colorBuffer);
                    }

                    for (const auto& buffer : vertexBuffers)
                    {
                        commands->transitionBarrior(buffer, GpuResourceState::copyDestination, GpuResourceState::vertexBuffer);
                    }

                    commands->close();

                    const auto commandExe = { commands };
                    device.getCommandQueue()->executeCommands(commandExe);
                    device.reuseCommandAllocatorAfterExecution(allocator);
                    device.reuseCommandListAfterExecution(commands);

                    const Resource<Material> material(resources, "media/box.material");
                    parsedMesh->createSubmesh(fbxMesh->GetName(), vertexData, material);
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

    std::shared_ptr<Mesh> FbxMeshImporter::import(RenderDevice& device, ResourceManager& resources, const std::string& path)
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

        return parseMeshScene(device, resources, scene->GetRootNode());
    }
}