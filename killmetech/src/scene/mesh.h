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

    /** Submesh */
    class Submesh
    {
    private:
        std::shared_ptr<VertexData> vertexData_;
        Resource<Material> material_;

    public:
        /** Construct with a vertices and a material */
        Submesh(const std::shared_ptr<VertexData>& vertexData, const Resource<Material>& material)
            : vertexData_(vertexData)
            , material_(material)
        {}

        /** Return the vertices */
        std::shared_ptr<VertexData> getVertexData() { return vertexData_; }

        /** Return the material */
        std::shared_ptr<Material> getMaterial() const { return material_.access(); }
    };

    /** Mesh */
    /**
     *  NOTE: Typically, the mesh means the 3D model. However in the KillMeTech, the mesh is a simple set
     *        of sub meshes that are pair of vertices and material. This means the abstract of the 
     *        mesh is a generic rendering object including the 3D model.
     */
    class Mesh : public IsResource
    {
    private:
        using Pair = std::pair<std::string, std::shared_ptr<Submesh>>;
        std::vector<Pair> submeshes_;

    public:
        /** Create a submesh */
        std::shared_ptr<Submesh> createSubmesh(const std::string& name, const std::shared_ptr<VertexData>& vertexData, const Resource<Material>& material)
        {
            const auto sm = std::make_shared<Submesh>(vertexData, material);
            submeshes_.emplace_back(name, sm);
            return sm;
        }

        /** Return the submesh */
        std::shared_ptr<Submesh> findSubmesh(const std::string& name)
        {
            const auto begin = std::cbegin(submeshes_);
            const auto end = std::cend(submeshes_);
            const auto it = std::find_if(begin, end, [&](const Pair& pair) { return pair.first == name; });
            if (it == end)
            {
                return nullptr;
            }
            return it->second;
        }

        /** Return the range of submeshes */
        auto getSubmeshes()
            -> decltype(constRange(submeshes_))
        {
            return constRange(submeshes_);
        }
    };
}

#endif