#include "graphicssystem.h"
#include "../renderer/renderdevice.h"
#include "../renderer/commandlist.h"
#include "../renderer/gpuresource.h"
#include "../renderer/commandqueue.h"
#include "../core/math/color.h"

namespace killme
{
    GraphicsSystem graphicsSystem;

    void GraphicsSystem::startup(HWND window)
    {
        RECT clientRect;
        GetClientRect(window, &clientRect);

        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        clientViewport_.topLeftX = 0;
        clientViewport_.topLeftY = 0;
        clientViewport_.width = static_cast<float>(clientWidth);
        clientViewport_.height = static_cast<float>(clientHeight);
        clientViewport_.minDepth = 0;
        clientViewport_.maxDepth = 1;

        renderSystem_ = std::make_unique<RenderSystem>(window);
    }

    void GraphicsSystem::shutdown()
    {
        renderSystem_.reset();
    }

    RenderSystem& GraphicsSystem::getRenderSystem()
    {
        return *renderSystem_;
    }

    RenderDevice& GraphicsSystem::getDevice()
    {
        return *renderSystem_->getDevice();
    }

    FrameResource GraphicsSystem::getCurrentFrameResource()
    {
        return renderSystem_->getCurrentFrameResource();
    }

    Viewport GraphicsSystem::getClientViewport() const
    {
        return clientViewport_;
    }

    void GraphicsSystem::clearBackBuffer()
    {
        const auto device = renderSystem_->getDevice();
        const auto frame = getCurrentFrameResource();
        const auto allocator = device->obtainCommandAllocator();
        const auto commands = device->obtainCommandList(allocator, nullptr);
        commands->transitionBarrior(frame.backBuffer, GpuResourceState::present, GpuResourceState::renderTarget);
        commands->clearRenderTarget(frame.backBufferLocation, { 0.1f, 0.1f, 0.1f, 1 });
        commands->transitionBarrior(frame.backBuffer, GpuResourceState::renderTarget, GpuResourceState::present);
        commands->clearDepthStencil(frame.depthStencilLocation, 1);
        commands->close();

        const auto commandExe = { commands };
        device->getCommandQueue()->executeCommands(commandExe);
        device->reuseCommandAllocatorAfterExecution(allocator);
        device->reuseCommandListAfterExecution(commands);
    }

    void GraphicsSystem::presentBackBuffer()
    {
        renderSystem_->presentBackBuffer();
    }
}