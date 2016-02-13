#ifndef _KILLME_MESHENTITY_H_
#define _KILLME_MESHENTITY_H_

#include "sceneentity.h"
#include "scenevisitor.h"
#include "../resource/resource.h"
#include <memory>

namespace killme
{
    class Mesh;

    /** The meshed model entity */
    class MeshEntity : public SceneEntity, public std::enable_shared_from_this<MeshEntity>
    {
    private:
        Resource<Mesh> mesh_;

    public:
        /** Constructs */
        MeshEntity(const Resource<Mesh>& mesh) : mesh_(mesh) {}

        /** Returns the mesh */
        Resource<Mesh> getMesh() { return mesh_; }

        bool accept(SceneVisitor& v) { return v(lockOwner(), shared_from_this()); }
    };
}

#endif