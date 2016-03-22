#ifndef _KILLME_DEBUGDRAWMANAGER_H_
#define _KILLME_DEBUGDRAWMANAGER_H_

#include "../renderer/renderstate.h"
#include "../resources/resource.h"
#include <memory>
#include <vector>

namespace killme
{
    class Vector3;
    class Color;
    class RenderDevice;
    class RenderSystem;
    class Material;
    class Camera;
    class ResourceManager;
    struct FrameResource;

    /** Debug drawer */
    class DebugDrawManager
    {
    private:
        std::vector<Vector3> positions_;
        std::vector<Color> colors_;

        std::shared_ptr<RenderDevice> device_;
        ScissorRect scissorRect_;
        Resource<Material> material_;

    public:
        /** Initialize */
        void initialize(RenderSystem& renderSystem, ResourceManager& resources);

        /** Finalize */
        void finalize();

        /** Add line */
        void line(const Vector3& from, const Vector3& to, const Color& color);

        /** Clear all debugs */
        void clear();

        /** Draw */
        void debugDraw(const Camera& camera, const FrameResource& frame);
    };

    extern DebugDrawManager debugDrawManager;
}

#endif