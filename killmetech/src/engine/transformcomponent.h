#ifndef _KILLME_TRANSFORMCOMPONENT_H_
#define _KILLME_TRANSFORMCOMPONENT_H_

#include "actorcomponent.h"
#include "graphics.h"
#include "processes.h"
#include "../processes/process.h"
#include "../scene/scenenode.h"
#include "../core/math/transform.h"
#include "../core/utility.h"
#include "../core/platform.h"
#include <memory>
#include <assert.h>

namespace killme
{
    class SceneNode;

    /** The transform component adds transform into an actor that becomes placeable in the world. */
    /// TODO: Attach to scene
    class TransformComponent : public ActorComponent, public Transform<TransformComponent>
    {
    private:
        std::shared_ptr<SceneNode> sceneNode_;
        Process process_;

    public:
        TransformComponent()
            : sceneNode_(std::make_shared<SceneNode>())
            , process_()
        {
        }

#ifdef KILLME_DEBUG
        void addChild(const std::shared_ptr<TransformComponent>& child)
        {
            // Transform should be active (attached).
            assert(lockOwner() && "The negative transform has not children.");
            assert(lockOwner() == child->lockOwner() && "Invalid transform tree.");
            Transform<TransformComponent>::addChild(child);
        }
#endif

        void onAttached()
        {
            // Transform should be leaf
            assert(getNumChildren() == 0 && "Transform should be leaf when attached.");

            if (const auto parent = lockParent())
            {
                // This is not root transform
                parent->sceneNode_->addChild(sceneNode_);
            }
            else
            {
                // This is root transform
                Graphics::addSceneNode(sceneNode_);
            }

            process_ = Processes::start([&] { tickScene(); }, PROCESS_PRIORITY_SCENE);
        }

        void onDettached()
        {
            if (const auto parent = lockParent())
            {
                // This is not root transform
                parent->removeChild(shared_from_this());
                for (const auto& c : getChildren())
                {
                    parent->addChild(c);
                }

                parent->sceneNode_->removeChild(sceneNode_);
                for (const auto& c : sceneNode_->getChildren())
                {
                    parent->sceneNode_->addChild(c);
                }
            }
            else
            {
                // This is root transform that should be no children
                assert(getNumChildren() == 0 && "Invalid dettach order.");
            }

            process_.kill();
        }

    protected:
        /** Returns scene node */
        std::shared_ptr<SceneNode> getSceneNode()
        {
            return sceneNode_;
        }

    private:
        void tickScene()
        {
            sceneNode_->setPosition(getPosition());
            sceneNode_->setOrientation(getOrientation());
            sceneNode_->setScale(getScale());
        }
    };
}

#endif