#include "graphics.h"
#include "resources.h"
#include "../import/fbxmeshimporter.h"
#include "../scene/scene.h"
#include "../scene/scenenode.h"
#include "../scene/material.h"
#include "../scene/mesh.h"
#include "../renderer/rendersystem.h"
#include "../renderer/renderstate.h"
#include "../renderer/shader.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../renderer/texture.h"
#include "../core/string.h"

namespace killme
{
    namespace
    {
        std::shared_ptr<RenderSystem> renderSystem;
        std::unique_ptr<FbxMeshImporter> fbxImporter;
        std::unique_ptr<Scene> scene;
        std::shared_ptr<Camera> mainCamera;
        Viewport clientViewport;
    }

    void Graphics::startup(HWND window)
    {
        RECT clientRect;
        GetClientRect(window, &clientRect);

        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        clientViewport.topLeftX = 0;
        clientViewport.topLeftY = 0;
        clientViewport.width = static_cast<float>(clientWidth);
        clientViewport.height = static_cast<float>(clientHeight);
        clientViewport.minDepth = 0;
        clientViewport.maxDepth = 1;

        renderSystem = std::make_shared<RenderSystem>(window);
        scene = std::make_unique<Scene>(renderSystem);
        fbxImporter = std::make_unique<FbxMeshImporter>();

        Resources::registerLoader("vhlsl", [](const std::string& path)      { return compileShader<VertexShader>(toCharSet(path)); });
        Resources::registerLoader("phlsl", [](const std::string& path)      { return compileShader<PixelShader>(toCharSet(path)); });
        Resources::registerLoader("material", [&](const std::string& path)  { return loadMaterial(renderSystem, Resources::getManager(), path); });
        Resources::registerLoader("bmp", [&](const std::string& path)      { return loadTextureFromBmp(*renderSystem, path); });
        Resources::registerLoader("fbx", [&](const std::string& path)       { return fbxImporter->import(*renderSystem, Resources::getManager(), path); });
    }

    void Graphics::shutdown()
    {
        Resources::unregisterLoader("fbx");
        Resources::unregisterLoader("bmp");
        Resources::unregisterLoader("material");
        Resources::unregisterLoader("phlsl");
        Resources::unregisterLoader("vhlsl");

        mainCamera.reset();
        fbxImporter.reset();
        scene.reset();
        renderSystem.reset();
    }

    void Graphics::setAmbientLight(const Color& c)
    {
        scene->setAmbientLight(c);
    }
    
    void Graphics::addLight(const std::shared_ptr<Light>& light)
    {
        scene->addLight(light);
    }
    
    void Graphics::removeLight(const std::shared_ptr<Light>& light)
    {
        scene->removeLight(light);
    }

    std::shared_ptr<RenderSystem> Graphics::getRenderSystem()
    {
        return renderSystem;
    }

    Viewport Graphics::getClientViewport()
    {
        return clientViewport;
    }

    std::shared_ptr<Camera> Graphics::getMainCamera()
    {
        return mainCamera;
    }

    void Graphics::setMainCamera(const std::shared_ptr<Camera>& camera)
    {
        mainCamera = camera;
    }

    void Graphics::addSceneNode(const std::shared_ptr<SceneNode>& node)
    {
        scene->getRootNode()->addChild(node);
    }

    void Graphics::renderScene()
    {
        if (mainCamera)
        {
            scene->renderScene(*mainCamera);
        }
    }

    void Graphics::presentBackBuffer()
    {
        renderSystem->presentBackBuffer();
    }
}