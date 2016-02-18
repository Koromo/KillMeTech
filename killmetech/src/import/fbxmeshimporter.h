#ifndef _KILLME_FBXMESHIMPORTER_H_
#define _KILLME_FBXMESHIMPORTER_H_

#include "fbxsupport.h"
#include <fbxsdk.h>
#include <memory>
#include <string>

namespace killme
{
    class Mesh;
    class RenderSystem;
    class ResourceManager;

    /** The fbx mesh importer */
    class FbxMeshImporter
    {
    private:
        FbxUniquePtr<FbxManager> fbxManager_;

    public:
        /** Constructs */
        FbxMeshImporter();

        /** Imports a mesh */
        std::shared_ptr<Mesh> import(RenderSystem& renderSystem, ResourceManager& resourceManager, const std::string& path);
    };
}

#endif