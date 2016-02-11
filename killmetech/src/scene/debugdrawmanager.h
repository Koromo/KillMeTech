#ifndef _KILLME_DEBUGDRAWMANAGER_H_
#define _KILLME_DEBUGDRAWMANAGER_H_

#include "../renderer/renderstate.h"
#include "../core/platform.h"
#include <memory>
#include <vector>

namespace killme
{
    class Vector3;
    class Color;
    class PipelineState;
    class CommandList;
    class ConstantBuffer;
    class GpuResourceHeap;

#ifdef KILLME_DEBUG

    class DebugDrawManager
    {
    private:
        std::vector<Vector3> positions_;
        std::vector<Color> colors_;

        std::shared_ptr<ConstantBuffer> viewProjBuffer_;
        std::shared_ptr<GpuResourceHeap> viewProjHeap_;
        std::shared_ptr<PipelineState> pipeline_;
        std::shared_ptr<CommandList> commandList_;
        ScissorRect scissorRect_;

    public:
        void startup();
        void shutdown();
        void line(const Vector3& from, const Vector3& to, const Color& color);
        void drawDebugs();
    };

#else

    class DebugDrawManager
    {
    public:
        void startup();
        void shutdown();
        void line(const Vector3& from, const Vector3& to, const Color& color);
        void drawDebugs();
    };

#endif

    extern DebugDrawManager debugDrawManager;
}

#endif