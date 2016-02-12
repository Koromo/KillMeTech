#include "audiomanager.h"
#include "audio3d.h"
#include "sourcevoice.h"
#include "audioclip.h"
#include "../resource/resourcemanager.h"
#include "../core/exception.h"
#include "../core/string.h"

namespace killme
{
    AudioManager audioManager;

    void AudioManager::startup()
    {
        // Initialize COM
        enforce<XAudioException>(
            SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)),
            "Failed to initialize COM Library."
            );

        // Initialize XAudio2
        IXAudio2* xAudio;
        enforce<XAudioException>(
            SUCCEEDED(XAudio2Create(&xAudio)),
            "Failed to create IXAudio2."
            );
        xAudio_ = makeComUnique(xAudio);

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

        audio3D.startup(channelMask, details.InputChannels);

        /** Set audio resource loaders */
        resourceManager.setLoader("wav", [](const std::string& path) { return loadAudioClip(toCharSet(path)); });
    }
    
    void AudioManager::shutdown()
    {
        audio3D.shutdown();
        xAudio_->StopEngine();
        masteringVoice_.reset();
        xAudio_.reset();
        CoUninitialize();
    }
    
    bool AudioManager::isActive() const
    {
        return !!xAudio_;
    }

    std::shared_ptr<SourceVoice> AudioManager::createSourceVoice(const Resource<AudioClip>& clip)
    {
        return std::make_shared<SourceVoice>(xAudio_.get(), clip);
    }
}