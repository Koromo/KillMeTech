#ifndef _KILLME_MESHCOMPONENT_H_
#define _KILLME_MESHCOMPONENT_H_

#include "transformcomponent.h"
#include <string>

namespace killme
{
    class Mesh;

    /** The mesh component adds rendered mesh into an actor */
    class MeshComponent : public TransformComponent
    {
    public:
        /** Constructs */
        explicit MeshComponent(const std::string& path);
    };
}

#endif