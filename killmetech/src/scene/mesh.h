#ifndef _KILLME_MESH_H_
#define _KILLME_MESH_H_

#include <memory>

namespace killme
{
    class VertexData;
    class Material;

    /** The mesh */
    class Mesh
    {
    private:
        std::shared_ptr<VertexData> vertexData_;
        std::shared_ptr<Material> material_;

    public:
        /** Constructs with a vertices and a material */
        Mesh(const std::shared_ptr<VertexData>& vertexData, const std::shared_ptr<Material>& material)
            : vertexData_(vertexData)
            , material_(material)
        {}

        /** Returns the vertices */
        std::shared_ptr<VertexData> getVertexData() { return vertexData_; }

        /** Returns the material */
        std::shared_ptr<Material> getMaterial() const { return material_; }
    };
}

#endif