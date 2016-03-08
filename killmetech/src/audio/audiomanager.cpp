#include "audiomanager.h"
#include "sourcevoice.h"
#include "../core/exception.h"

namespace killme
{
    AudioManager::AudioManager()
        : xAudio_()
        , masteringVoice_()
        , deviceDetails_()
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

        DWORD channelMask;
        masteringVoice_->GetChannelMask(&channelMask);

        XAUDIO2_VOICE_DETAILS details;
        masteringVoice_->GetVoiceDetails(&details);

        deviceDetails_.channelMask = channelMask;
        deviceDetails_.numSrcCannels = details.InputChannels;
        deviceDetails_.numDestCannels = 2; /// TODO:
    }
    
    AudioManager::~AudioManager()
    {
        xAudio_->StopEngine();
        masteringVoice_.reset();
        xAudio_.reset();
    }

    AudioDeviceDetails AudioManager::getDeviceDetails() const
    {
        return deviceDetails_;
    }

    std::shared_ptr<SourceVoice> AudioManager::createSourceVoice(const Resource<AudioClip>& clip)
    {
        return std::make_shared<SourceVoice>(xAudio_, clip);
    }
}