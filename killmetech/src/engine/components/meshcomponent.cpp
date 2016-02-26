#include "meshcomponent.h"
#include "../resources.h"
#include "../../scene/scenenode.h"
#include "../../scene/meshentity.h"
#include "../../scene/mesh.h"

namespace killme
{
    MeshComponent::MeshComponent(const std::string& path)
        : entity_()
    {
        const auto meshNode = getSceneNode();
        entity_ = meshNode->attachEntity<MeshEntity>(Resources::load<Mesh>(path));
    }

    std::shared_ptr<SubMesh> MeshComponent::findSubMesh(const std::string& name)
    {
        return entity_->findSubMesh(name);
    }
}