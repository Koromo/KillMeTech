#include "audiosystem.h"
#include "../audio/audiomanager.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <Windows.h>

namespace killme
{
    AudioSystem audioSystem;

    void AudioSystem::startup()
    {
        enforce<WindowsException>(
            SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)),
            "Failed to initialize COM Library."
            );

        manager_ = std::make_unique<AudioManager>();
    }
    
    void AudioSystem::shutdown()
    {
        manager_.reset();
        CoUninitialize();
    }
    
    std::shared_ptr<SourceVoice> AudioSystem::createSourceVoice(const Resource<AudioClip>& clip)
    {
        return manager_->createSourceVoice(clip);
    }

    AudioDeviceDetails AudioSystem::getDeviceDetails() const
    {
        return manager_->getDeviceDetails();
    }

}