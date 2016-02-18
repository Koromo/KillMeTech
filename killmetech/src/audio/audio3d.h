#ifndef _KILLME_AUDIO3D_H_
#define _KILLME_AUDIO3D_H_

#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include <memory>

namespace killme
{
    class SourceVoice;

    /** The audio listener parameters */
    struct ListenerParams
    {
        Vector3 position;
        Quaternion orientation;
        Vector3 velocity;
    };

    /** The audio emitter parameters */
    struct EmitterParams
    {
        Vector3 position;
        Quaternion orientation;
        Vector3 velocity;
        std::shared_ptr<SourceVoice> voice;
    };
}

#endif