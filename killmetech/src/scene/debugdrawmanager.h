#ifndef _KILLME_DEBUGDRAWMANAGER_H_
#define _KILLME_DEBUGDRAWMANAGER_H_

#include "../renderer/renderstate.h"
#include <memory>
#include <vector>

namespace killme
{
    class Vector3;
    class Color;
    class PipelineState;
    class ConstantBuffer;
    class GpuResourceHeap;
    class RenderSystem;
    class Camera;
    struct FrameResource;

    /** Debug drawer */
    class DebugDrawManager
    {
    private:
        std::vector<Vector3> positions_;
        std::vector<Color> colors_;

        std::shared_ptr<RenderSystem> renderSystem_;
        std::shared_ptr<ConstantBuffer> viewProjBuffer_;
        std::shared_ptr<GpuResourceHeap> viewProjHeap_;
        std::shared_ptr<PipelineState> pipeline_;
        ScissorRect scissorRect_;

    public:
        /** Initialize */
        void initialize(const std::shared_ptr<RenderSystem>& renderSystem);

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