#ifndef _KILLME_MESHENTITY_H_
#define _KILLME_MESHENTITY_H_

#include "renderqueue.h"
#include "sceneentity.h"
#include "../resources/resource.h"
#include <memory>

namespace killme
{
    class Mesh;
    class SubMesh;
    class Material;
    class MeshRenderer;
    class SceneNode;
    class RenderQueue;
    struct SceneContext;

    /** Meshed model entity */
    class MeshEntity : public SceneEntity
    {
    private:
        Resource<Mesh> mesh_;
        std::shared_ptr<MeshRenderer> renderer_;

    public:
        /** Construct */
        MeshEntity(const Resource<Mesh>& mesh);

        /** Search sub mesh */
        std::shared_ptr<SubMesh> findSubMesh(const std::string& name);

        void collectRenderer(RenderQueue& queue);
    };

    /** Mesh renderer */
    class MeshRenderer : public Renderer
    {
    private:
        std::weak_ptr<SceneNode> node_;
        Resource<Mesh> mesh_;

    public:
        /** Construct */
        MeshRenderer(const Resource<Mesh>& mesh);

        /** Set the owner node */
        void setOwnerNode(const std::weak_ptr<SceneNode>& node);

        /** Execute render */
        void render(const SceneContext& context);
    };
}

#endif