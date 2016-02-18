#ifndef _KILLME_MESHCOMPONENT_H_
#define _KILLME_MESHCOMPONENT_H_

#include "transformcomponent.h"
#include "resources.h"
#include "processes.h"
#include "../processes/process.h"
#include "../scene/scenenode.h"
#include "../scene/meshentity.h"
#include <memory>

namespace killme
{
    class Mesh;

    /** The mesh component adds rendered mesh into an actor */
    class MeshComponent : public TransformComponent
    {
    public:
        explicit MeshComponent(const std::string& path)
        {
            const auto meshNode = getSceneNode();
            meshNode->attachEntity<MeshEntity>(Resources::load<Mesh>(path));
        }
    };
}

#endif