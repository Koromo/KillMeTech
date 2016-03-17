#include "resourcemanagesystem.h"
#include "graphicssystem.h"
#include "../import/fbxmeshimporter.h"
#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../scene/materialcreation.h"
#include "../audio/audioclip.h"
#include "../renderer/rendersystem.h"
#include "../renderer/shaders.h"
#include "../renderer/texture.h"
#include "../renderer/image.h"
#include "../renderer/pixels.h"
#include "../renderer/commandlist.h"
#include "../renderer/commandqueue.h"
#include "../renderer/resourcebarrior.h"
#include "../core/string.h"

namespace killme
{
    ResourceManageSystem resourceManager;

    ResourceManager& ResourceManageSystem::getManager()
    {
        return *manager_;
    }

    void ResourceManageSystem::startup()
    {
        manager_ = std::make_unique<ResourceManager>();
        fbxImporter_ = std::make_unique<FbxMeshImporter>();

        registerLoader("vhlsl", [](const std::string& path) { return compileHlslShader<VertexShader>(toCharSet(path)); });
        registerLoader("phlsl", [](const std::string& path) { return compileHlslShader<PixelShader>(toCharSet(path)); });
        registerLoader("ghlsl", [](const std::string& path) { return compileHlslShader<GeometryShader>(toCharSet(path)); });
        registerLoader("material", [&](const std::string& path){ return loadMaterial(*graphicsSystem.getRenderSystem(), getManager(), path); });
        registerLoader("bmp", [&](const std::string& path)
        {
            const auto renderSystem = graphicsSystem.getRenderSystem();
            const auto img = decodeBmpImage(path);
            const auto tex = renderSystem->createTexture(img->getWidth(), img->getHeight(), Pixel_r8g8b8a8::UNORM_FORMAT);

            const auto allocator = renderSystem->obtainCommandAllocator();
            const auto commands = renderSystem->obtainCommandList(allocator, nullptr);
            commands->updateGpuResource(tex, img->getPixels());
            commands->transitionBarrior(tex, ResourceState::copyDestination, ResourceState::texture);
            commands->close();

            const auto commandExe = { commands };
            renderSystem->getCommandQueue()->executeCommands(commandExe);
            renderSystem->reuseCommandAllocatorAfterExecution(allocator);
            renderSystem->reuseCommandListAfterExecution(commands);

            return tex;
        });
        registerLoader("fbx", [&](const std::string& path) { return fbxImporter_->import(*graphicsSystem.getRenderSystem(), getManager(), path); });
        registerLoader("wav", [](const std::string& path) { return loadWavAudio(toCharSet(path)); });
    }

    void ResourceManageSystem::shutdown()
    {
        unregisterLoader("wav");
        unregisterLoader("fbx");
        unregisterLoader("bmp");
        unregisterLoader("material");
        unregisterLoader("ghlsl");
        unregisterLoader("phlsl");
        unregisterLoader("vhlsl");

        fbxImporter_.reset();
        manager_.reset();
    }

    void ResourceManageSystem::registerLoader(const std::string& ext, ResourceLoader loader)
    {
        manager_->registerLoader(ext, loader);
    }

    void ResourceManageSystem::unregisterLoader(const std::string& ext)
    {
        manager_->unregisterLoader(ext);

    }
}