#ifndef _KILLME_MESHINSTANCE_H_
#define _KILLME_MESHINSTANCE_H_

#include "../resources/resource.h"
#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include "../core/math/matrix44.h"
#include <memory>

namespace killme
{
    class Mesh;

    /** Mesh instance */
    class MeshInstance
    {
    private:
        Resource<Mesh> mesh_;
        Vector3 position_;
        Quaternion orientation_;
        Vector3 scale_;

    public:
        /** Construct */
        explicit MeshInstance(const Resource<Mesh>& mesh)
            : mesh_(mesh)
            , position_()
            , orientation_()
            , scale_(1, 1, 1)
        {}

        /** Return the mesh */
        Resource<Mesh> getMesh() { return mesh_; }

        /** Transform modifiers */
        void setPosition(const Vector3& pos) { position_ = pos; }
        void setOrientation(const Quaternion& q) { orientation_ = q; }
        void setScale(const Vector3& k) { scale_ = k; }
        Matrix44 getWorldMatrix() const { return makeTransformMatrix(scale_, orientation_, position_); }
    };
}

#endif