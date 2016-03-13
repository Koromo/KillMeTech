#ifndef _KILLME_MESHCOMPONENT_H_
#define _KILLME_MESHCOMPONENT_H_

#include "transformcomponent.h"
#include "../../resources/resource.h"
#include <memory>
#include <string>

namespace killme
{
    class Mesh;
    class Submesh;
    class MeshInstance;

    /** The mesh component adds rendered mesh into an actor */
    class MeshComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE_BEGIN(MeshComponent)
        KILLME_COMPONENT_DEFINE_END

    private:
        std::shared_ptr<MeshInstance> inst_;

    public:
        /** Construct */
        explicit MeshComponent(const Resource<Mesh>& mesh);

        /** Return the submesh */
        std::shared_ptr<Submesh> findSubmesh(const std::string& name);

    private:
        void onTranslated();
        void onRotated();
        void onScaled();

        void onActivate();
        void onDeactivate();
    };
}

#endif