#ifndef _KILLME_MESHENTITY_H_
#define _KILLME_MESHENTITY_H_

#include "sceneentity.h"
#include "scenevisitor.h"
#include <memory>

namespace killme
{
    class Mesh;

    /** Meshed model entity */
    class MeshEntity : public SceneEntity, public std::enable_shared_from_this<MeshEntity>
    {
    private:
        std::shared_ptr<Mesh> mesh_;

    public:
        /** Construct */
        MeshEntity(const std::shared_ptr<Mesh>& mesh) : mesh_(mesh) {}

        /** Returns mesh */
        std::shared_ptr<Mesh> getMesh() { return mesh_; }

        bool accept(SceneVisitor& v) { return v(lockOwner(), shared_from_this()); }
    };
}

#endif