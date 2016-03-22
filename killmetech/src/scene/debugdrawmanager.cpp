#include "debugdrawmanager.h"
#include "material.h"
#include "effecttechnique.h"
#include "effectpass.h"
#include "materialcreation.h"
#include "camera.h"
#include "../renderer/renderdevice.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexdata.h"
#include "../renderer/commandlist.h"
#include "../renderer/commandqueue.h"
#include "../renderer/commandallocator.h"
#include "../renderer/pipelinestate.h"
#include "../core/math/matrix44.h"
#include <Windows.h>

namespace killme
{
    DebugDrawManager debugDrawManager;

    void DebugDrawManager::initialize(RenderSystem& renderSystem, ResourceManager& resources)
    {
        device_ = renderSystem.getDevice();
        material_ = Resource<Material>(resources, "media/debugdraw.material");

        const auto window = renderSystem.getTargetWindow();
        RECT clientRect;
        GetClientRect(window, &clientRect);

        scissorRect_.top = 0;
        scissorRect_.left = 0;
        scissorRect_.right = clientRect.right - clientRect.left;
        scissorRect_.bottom = clientRect.bottom - clientRect.top;
    }

    void DebugDrawManager::finalize()
    {
        material_.unload();
        device_.reset();
        clear();
    }

    void DebugDrawManager::line(const Vector3& from, const Vector3& to, const Color& color)
    {
        positions_.emplace_back(from);
        positions_.emplace_back(to);
        colors_.emplace_back(color);
        colors_.emplace_back(color);
    }

    void DebugDrawManager::clear()
    {
        positions_.clear();
        colors_.clear();
    }

    void DebugDrawManager::debugDraw(const Camera& camera, const FrameResource& frame)
    {
        const auto numVertices = positions_.size();
        if (numVertices == 0)
        {
            return;
        }

        // Prepare debug render resources
        const auto allocator = device_->obtainCommandAllocator();
        const auto commands = device_->obtainCommandList(allocator, nullptr);

        const auto positionBuffer = device_->createVertexBuffer(numVertices * sizeof(float) * 3, sizeof(float) * 3, GpuResourceState::copyDestination);
        const auto colorBuffer = device_->createVertexBuffer(numVertices * sizeof(float) * 4, sizeof(float) * 4, GpuResourceState::copyDestination);
        commands->updateGpuResource(positionBuffer, positions_.data());
        commands->updateGpuResource(colorBuffer, colors_.data());
        commands->transitionBarrior(positionBuffer, GpuResourceState::copyDestination, GpuResourceState::vertexBuffer);
        commands->transitionBarrior(colorBuffer, GpuResourceState::copyDestination, GpuResourceState::vertexBuffer);
        commands->close();

        {
            const auto commandExe = { commands };
            device_->getCommandQueue()->executeCommands(commandExe);
            device_->getCommandQueue()->waitForCommands();
        }

        const auto vertexData = std::make_shared<VertexData>();
        vertexData->addVertices(SemanticNames::position, 0, positionBuffer);
        vertexData->addVertices(SemanticNames::color, 0, colorBuffer);

        const auto viewport = camera.getViewport();
        const auto viewMat = transpose(camera.getViewMatrix());
        const auto projMat = transpose(camera.getProjectionMatrix());

        material_.access()->setNumeric("ViewMatrix", to<MP_float4x4>(viewMat));
        material_.access()->setNumeric("ProjMatrix", to<MP_float4x4>(projMat));

        const auto tech = material_.access()->getUseTechnique();
        const auto pass = (*std::cbegin(tech->getPasses()));
        const auto pipeline = pass->getPipelineState();
        pipeline->setRenderTarget(0, frame.backBufferLocation);
        pipeline->setDepthStencil(frame.depthStencilLocation);
        pipeline->setViewport(viewport);
        pipeline->setScissorRect(scissorRect_);
        pipeline->setPrimitiveTopology(PrimitiveTopology::lineList);
        pipeline->setVertexBuffers(vertexData);

        // Begin drawing to all debugs
        allocator->reset();
        commands->reset(allocator, pipeline);

        commands->transitionBarrior(frame.backBuffer, GpuResourceState::present, GpuResourceState::renderTarget);
        commands->draw(numVertices);
        commands->transitionBarrior(frame.backBuffer, GpuResourceState::renderTarget, GpuResourceState::present);
        commands->close();

        {
            const auto commandExe = { commands };
            device_->getCommandQueue()->executeCommands(commandExe);
        }
        
        device_->reuseCommandAllocatorAfterExecution(allocator);
        device_->reuseCommandListAfterExecution(commands);
        device_->getCommandQueue()->waitForCommands();

        clear();
    }
}