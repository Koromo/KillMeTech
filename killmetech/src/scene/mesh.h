#ifndef _KILLME_MESH_H_
#define _KILLME_MESH_H_

#include <memory>

namespace killme
{
    class VertexData;
    class Material;

    /** Mesh */
    class Mesh
    {
    private:
        std::shared_ptr<VertexData> vertexData_;
        std::shared_ptr<Material> material_;

    public:
        /** Construct with a vertices and material */
        Mesh(const std::shared_ptr<VertexData>& vertexData, const std::shared_ptr<Material>& material)
            : vertexData_(vertexData)
            , material_(material)
        {}

        /** Returns vertices */
        std::shared_ptr<VertexData> getVertexData() { return vertexData_; }

        /** Returns material */
        std::shared_ptr<Material> getMaterial() const { return material_; }
    };
}

#endif