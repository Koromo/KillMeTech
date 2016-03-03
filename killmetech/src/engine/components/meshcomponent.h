#ifndef _KILLME_MESHCOMPONENT_H_
#define _KILLME_MESHCOMPONENT_H_

#include "transformcomponent.h"
#include "../resources.h"
#include <memory>

namespace killme
{
    class Mesh;
    class SubMesh;
    class MeshEntity;

    /** The mesh component adds rendered mesh into an actor */
    class MeshComponent : public TransformComponent
    {
    private:
        std::shared_ptr<MeshEntity> entity_;

    public:
        /** Construct */
        explicit MeshComponent(const Resource<Mesh>& mesh);

        /** Return the sub mesh */
        std::shared_ptr<SubMesh> findSubMesh(const std::string& name);
    };
}

#endif