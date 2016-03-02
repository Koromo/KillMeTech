#ifndef _KILLME_TRANSFORMCOMPONENT_H_
#define _KILLME_TRANSFORMCOMPONENT_H_

#include "actorcomponent.h"
#include "../../processes/process.h"
#include "../../core/math/transform.h"
#include "../../core/platform.h"
#include <memory>

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
        /** Construct */
        TransformComponent();

#ifdef KILLME_DEBUG
        void addChild(const std::shared_ptr<TransformComponent>& child);
#endif

        void onAttached();
        void onDettached();

    protected:
        /** Return scene node */
        std::shared_ptr<SceneNode> getSceneNode();

    private:
        void tickScene();
    };
}

#endif