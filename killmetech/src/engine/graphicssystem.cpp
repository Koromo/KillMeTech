#include "graphicssystem.h"
#include "../renderer/commandlist.h"
#include "../renderer/rendertarget.h"
#include "../renderer/commandlist.h"
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

        renderSystem_ = std::make_shared<RenderSystem>(window);
    }

    void GraphicsSystem::shutdown()
    {
        renderSystem_.reset();
    }

    std::shared_ptr<RenderSystem> GraphicsSystem::getRenderSystem()
    {
        return renderSystem_;
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
        const auto frame = getCurrentFrameResource();
        const auto commands = renderSystem_->beginCommands(nullptr);
        commands->resourceBarrior(frame.backBuffer, ResourceState::present, ResourceState::renderTarget);
        commands->clearRenderTarget(frame.backBufferView, { 0.1f, 0.1f, 0.1f, 1 });
        commands->resourceBarrior(frame.backBuffer, ResourceState::renderTarget, ResourceState::present);
        commands->clearDepthStencil(frame.depthStencilView, 1);
        commands->close();
        renderSystem_->executeCommands(commands);
    }

    void GraphicsSystem::presentBackBuffer()
    {
        renderSystem_->presentBackBuffer();
    }
}