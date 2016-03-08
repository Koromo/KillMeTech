#include "meshcomponent.h"
#include "../level.h"
#include "../../scene/meshinstance.h"
#include "../../scene/mesh.h"
#include "../../scene/scene.h"

namespace killme
{
    MeshComponent::MeshComponent(const Resource<Mesh>& mesh)
        : inst_(std::make_shared<MeshInstance>(mesh))
    {
        enableReceiveMove(true);
    }

    std::shared_ptr<Submesh> MeshComponent::findSubmesh(const std::string& name)
    {
        return inst_->getMesh().access()->findSubmesh(name);
    }

    void MeshComponent::onTranslated()
    {
        inst_->setPosition(getWorldPosition());
    }

    void MeshComponent::onRotated()
    {
        inst_->setOrientation(getWorldOrientation());
    }

    void MeshComponent::onScaled()
    {
        inst_->setScale(getWorldScale());
    }

    void MeshComponent::onActivate()
    {
        getOwnerLevel().getGraphicsWorld().addMeshInstance(inst_);
    }

    void MeshComponent::onDeactivate()
    {
        getOwnerLevel().getGraphicsWorld().removeMeshInstance(inst_);
    }
}