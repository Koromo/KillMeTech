#ifndef _KILLME_AUDIO3D_H_
#define _KILLME_AUDIO3D_H_

#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include <vector>
#include <Windows.h>
#include <x3daudio.h>
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

    /** The 3D audio manager */
    class Audio3D
    {
    private:
        X3DAUDIO_HANDLE x3DAudio_;
        X3DAUDIO_LISTENER listener_;
        size_t numSrcCannels_;
        size_t numDestCannels_;
        std::vector<float> levelMatrix_;

    public:
        /** Initializes */
        void startup(unsigned channelMask, size_t numSrcChannels);

        /** Finalizes */
        void shutdown();

        /** Sets the listener parameters */
        void setListener(const ListenerParams& params);

        /** Calculates the 3D audio */
        void calculate(const EmitterParams& params);
    };

    extern Audio3D audio3D;
}

#endif