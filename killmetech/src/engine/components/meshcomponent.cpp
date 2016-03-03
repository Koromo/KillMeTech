#include "meshcomponent.h"
#include "../../scene/scenenode.h"
#include "../../scene/meshentity.h"
#include "../../scene/mesh.h"

namespace killme
{
    MeshComponent::MeshComponent(const Resource<Mesh>& mesh)
        : entity_()
    {
        const auto meshNode = getSceneNode();
        entity_ = meshNode->attachEntity<MeshEntity>(mesh);
    }

    std::shared_ptr<SubMesh> MeshComponent::findSubMesh(const std::string& name)
    {
        return entity_->findSubMesh(name);
    }
}