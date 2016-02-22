#include "audioengine.h"
#include "sourcevoice.h"
#include "audioclip.h"
#include "audio3d.h"
#include "../core/exception.h"
#include "../core/string.h"
#include "../core/utility.h"
#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include <algorithm>

namespace killme
{
    AudioEngine::AudioEngine()
        : xAudio_()
        , masteringVoice_()
        , x3DAudio_()
        , listener_()
        , numSrcCannels_(0)
        , numDestCannels_(0)
        , levelMatrix_()
    {
        // Initialize XAudio2
        IXAudio2* xAudio;
        enforce<XAudioException>(
            SUCCEEDED(XAudio2Create(&xAudio)),
            "Failed to create IXAudio2."
            );
        xAudio_ = makeComShared(xAudio);

        IXAudio2MasteringVoice* masteringVoice;
        enforce<XAudioException>(
            SUCCEEDED(xAudio_->CreateMasteringVoice(&masteringVoice)),
            "Failed to create IXAudio2MasteringVoice."
            );
        masteringVoice_ = makeVoiceUnique(masteringVoice);

        // Initialize the 3D audio
        DWORD channelMask;
        masteringVoice_->GetChannelMask(&channelMask);

        XAUDIO2_VOICE_DETAILS details;
        masteringVoice_->GetVoiceDetails(&details);

        numSrcCannels_ = details.InputChannels;
        numDestCannels_ = 2; /// TODO:
        levelMatrix_.resize(numSrcCannels_ * numDestCannels_, 0);

        enforce<XAudioException>(
            SUCCEEDED(X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3DAudio_)),
            "Failed to initialize the X3DAudio.");

        ZeroMemory(&listener_, sizeof(listener_));
    }
    
    AudioEngine::~AudioEngine()
    {
        xAudio_->StopEngine();
        masteringVoice_.reset();
        xAudio_.reset();
    }

    std::shared_ptr<SourceVoice> AudioEngine::createSourceVoice(const Resource<AudioClip>& clip)
    {
        return std::make_shared<SourceVoice>(xAudio_, clip);
    }

    namespace
    {
        X3DAUDIO_VECTOR toX3DAudioVector(const Vector3& v)
        {
            return{ v.x, v.y, v.z };

        }
    }

    void AudioEngine::set3DListener(const ListenerParams& params)
    {
        listener_.OrientFront = toX3DAudioVector(params.orientation * Vector3::UNIT_Z);
        listener_.OrientTop = toX3DAudioVector(params.orientation * Vector3::UNIT_Y);
        listener_.Position = toX3DAudioVector(params.position);
        listener_.Velocity = toX3DAudioVector(params.velocity);
    }

    void AudioEngine::apply3DEmission(const EmitterParams& params)
    {
        X3DAUDIO_EMITTER emitter;
        ZeroMemory(&emitter, sizeof(emitter));
        emitter.OrientFront = toX3DAudioVector(params.orientation * Vector3::UNIT_Z);
        emitter.OrientTop = toX3DAudioVector(params.orientation * Vector3::UNIT_Y);
        emitter.Position = toX3DAudioVector(params.position);
        emitter.Velocity = toX3DAudioVector(params.velocity);

        /// TODO:
        emitter.InnerRadius = 30;
        emitter.ChannelCount = 1;
        emitter.pVolumeCurve = const_cast<X3DAUDIO_DISTANCE_CURVE*>(&X3DAudioDefault_LinearCurve);
        emitter.CurveDistanceScaler = 500;
        emitter.DopplerScaler = 1;

        X3DAUDIO_DSP_SETTINGS dsp;
        dsp.pMatrixCoefficients = levelMatrix_.data();
        dsp.pDelayTimes = NULL;
        dsp.SrcChannelCount = numSrcCannels_;
        dsp.DstChannelCount = numDestCannels_;

        X3DAudioCalculate(x3DAudio_, &listener_, &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dsp);

        params.voice->applyFrequencyRatio(dsp.DopplerFactor);
        params.voice->applyOutputMatrix(numSrcCannels_, numDestCannels_, levelMatrix_.data());
    }

    void AudioEngine::apply3DSilent(const std::shared_ptr<SourceVoice>& voice)
    {
        std::fill(std::begin(levelMatrix_), std::end(levelMatrix_), 0.0f);
        voice->applyOutputMatrix(numSrcCannels_, numDestCannels_, levelMatrix_.data());
    }
}