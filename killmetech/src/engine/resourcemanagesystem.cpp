#include "resourcemanagesystem.h"
#include "graphicssystem.h"
#include "../import/fbxmeshimporter.h"
#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../scene/materialcreation.h"
#include "../audio/audioclip.h"
#include "../renderer/rendersystem.h"
#include "../renderer/renderdevice.h"
#include "../renderer/shaders.h"
#include "../renderer/texture.h"
#include "../renderer/image.h"
#include "../renderer/pixels.h"
#include "../renderer/commandlist.h"
#include "../renderer/commandqueue.h"
#include "../renderer/gpuresource.h"
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
        registerLoader("material", [&](const std::string& path){ return loadMaterial(graphicsSystem.getDevice(), getManager(), path); });
        registerLoader("bmp", [&](const std::string& path)
        {
            auto& device = graphicsSystem.getDevice();
            const auto img = decodeBmpImage(path);
            const auto tex = device.createTexture(img->getWidth(), img->getHeight(), Pixel_r8g8b8a8::UNORM_FORMAT, TextureFlags::none, GpuResourceState::copyDestination, nullopt);

            const auto allocator = device.obtainCommandAllocator();
            const auto commands = device.obtainCommandList(allocator, nullptr);
            commands->updateGpuResource(tex, img->getPixels());
            commands->transitionBarrior(tex, GpuResourceState::copyDestination, GpuResourceState::texture);
            commands->close();

            const auto commandExe = { commands };
            device.getCommandQueue()->executeCommands(commandExe);
            device.reuseCommandAllocatorAfterExecution(allocator);
            device.reuseCommandListAfterExecution(commands);

            return tex;
        });
        registerLoader("fbx", [&](const std::string& path) { return fbxImporter_->import(graphicsSystem.getDevice(), getManager(), path); });
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