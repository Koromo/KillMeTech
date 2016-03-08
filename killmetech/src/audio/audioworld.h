#ifndef _KILLME_AUDIOWORLD_H_
#define _KILLME_AUDIOWORLD_H_

#define _XM_NO_INTRINSICS_ 1; // For x3daudio.h

#include "audiomanager.h"
#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include <Windows.h>
#include <x3daudio.h>
#include <vector>
#include <unordered_set>
#include <memory>

namespace killme
{
    class SourceVoice;

    /** Audio listener parameters */
    class AudioListener
    {
    public:
        Vector3 position;
        Quaternion orientation;
        Vector3 velocity;
    };

    /** Audio emitter parameters */
    class AudioEmitter
    {
    public:
        Vector3 position;
        Quaternion orientation;
        Vector3 velocity;
        std::shared_ptr<SourceVoice> voice;
    };

    /** Audio world */
    class AudioWorld
    {
    private:
        X3DAUDIO_HANDLE x3DAudio_;
        AudioDeviceDetails deviceDetails_;
        std::vector<float> levelMatrix_;

        std::unordered_set<std::shared_ptr<AudioListener>> listeners_;
        std::unordered_set<std::shared_ptr<AudioEmitter>> emitters_;
        std::shared_ptr<AudioListener> mainListener_;

    public:
        /** Construct */
        explicit AudioWorld(const AudioDeviceDetails& deviceDetails);

        /** Set the main listener */
        void setMainListener(const std::shared_ptr<AudioListener>& listener);

        /** Return the main listener */
        std::shared_ptr<AudioListener> getMainListener();

        /** Add a listener */
        void addListener(const std::shared_ptr<AudioListener>& listener);

        /** Remove a listener */
        void removeListener(const std::shared_ptr<AudioListener>& listener);

        /** Add an emitter */
        void addEmitter(const std::shared_ptr<AudioEmitter>& emitter);

        /** Remove an emitter */
        void removeEmitter(const std::shared_ptr<AudioEmitter>& emitter);

        /** Simulate 3D audio */
        void simulate();
    };
}

#endif