#ifndef _KILLME_MESH_H_
#define _KILLME_MESH_H_

#include "../resources/resource.h"
#include "../core/utility.h"
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>

namespace killme
{
    class VertexData;
    class Material;

    /** The sub mesh */
    class SubMesh
    {
    private:
        std::shared_ptr<VertexData> vertexData_;
        Resource<Material> material_;

    public:
        /** Constructs with a vertices and a material */
        SubMesh(const std::shared_ptr<VertexData>& vertexData, const Resource<Material>& material)
            : vertexData_(vertexData)
            , material_(material)
        {}

        /** Returns the vertices */
        std::shared_ptr<VertexData> getVertexData() { return vertexData_; }

        /** Returns the material */
        std::shared_ptr<Material> getMaterial() const { return material_.access(); }
    };

    /** The mesh */
    class Mesh : public IsResource
    {
    private:
        using Pair = std::pair<std::string, std::shared_ptr<SubMesh>>;
        std::vector<Pair> subMeshes_;

    public:
        /** Creates the sub mesh */
        std::shared_ptr<SubMesh> createSubMesh(const std::string& name, const std::shared_ptr<VertexData>& vertexData, const Resource<Material>& material)
        {
            const auto sm = std::make_shared<SubMesh>(vertexData, material);
            subMeshes_.emplace_back(name, sm);
            return sm;
        }

        /** Returns the sub mesh */
        std::shared_ptr<SubMesh> findSubMesh(const std::string& name)
        {
            const auto begin = std::cbegin(subMeshes_);
            const auto end = std::cend(subMeshes_);
            const auto it = std::find_if(begin, end, [&](const Pair& pair) { return pair.first == name; });
            if (it == end)
            {
                return nullptr;
            }
            return it->second;
        }

        /** Returns the range of sub meshes */
        auto getSubMeshes()
            -> decltype(makeRange(subMeshes_))
        {
            return makeRange(subMeshes_);
        }
    };
}

#endif