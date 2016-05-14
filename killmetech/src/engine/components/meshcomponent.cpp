#include "meshcomponent.h"
#include "../level.h"
#include "../../scene/meshinstance.h"
#include "../../scene/scene.h"

namespace killme
{
    MeshComponent::MeshComponent(const Resource<Mesh>& mesh)
        : inst_(std::make_shared<MeshInstance>(mesh))
    {
        setMoveRecievable(true);
    }

    Resource<Mesh> MeshComponent::getMesh()
    {
        return inst_->getMesh();
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