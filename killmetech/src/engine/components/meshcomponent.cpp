#include "meshcomponent.h"
#include "../resources.h"
#include "../../scene/scenenode.h"
#include "../../scene/meshentity.h"
#include "../../scene/mesh.h"

namespace killme
{
    MeshComponent::MeshComponent(const std::string& path)
    {
        const auto meshNode = getSceneNode();
        meshNode->attachEntity<MeshEntity>(Resources::load<Mesh>(path));
    }
}