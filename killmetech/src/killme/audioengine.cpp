#include "audioengine.h"
#include "exception.h"
#include "sourcevoice.h"

namespace killme
{
    AudioEngine::AudioEngine()
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
    }
    
    AudioEngine::~AudioEngine()
    {
        xAudio_->StopEngine();
        masteringVoice_.reset();
        xAudio_.reset();
        CoUninitialize();
    }
    
    std::shared_ptr<SourceVoice> AudioEngine::createSourceVoice(const std::shared_ptr<const AudioClip>& clip)
    {
        return std::make_shared<SourceVoice>(xAudio_, clip);
    }
}