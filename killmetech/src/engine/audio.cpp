#include "audio.h"
#include "resources.h"
#include "../audio/audioengine.h"
#include "../audio/audioclip.h"
#include "../audio/audio3d.h"
#include "../audio/xaudiosupport.h"
#include "../core/exception.h"
#include "../core/string.h"
#include <Windows.h>

namespace killme
{
    class AudioClip;

    namespace
    {
        AudioEngine* engine;
        bool noListener;
    }
    
    void Audio::startup()
    {
        enforce<XAudioException>(
            SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)),
            "Failed to initialize COM Library."
            );

        engine = new AudioEngine();

        Resources::registerLoader("wav", [](const std::string& path) { return loadWavAudio(toCharSet(path)); });
        noListener = true;
    }
    
    void Audio::shutdown()
    {
        Resources::unregisterLoader("wav");
        delete engine;
        CoUninitialize();
    }
    
    std::shared_ptr<SourceVoice> Audio::createSourceVoice(const Resource<AudioClip>& clip)
    {
        return engine->createSourceVoice(clip);
    }
    
    void Audio::set3DListener(const ListenerParams& params)
    {
        engine->set3DListener(params);
        noListener = false;
    }
    
    void Audio::apply3DEmission(const EmitterParams& params)
    {
        if (noListener)
        {
            engine->apply3DSilent(params.voice);
        }
        else
        {
            engine->apply3DEmission(params);
        }
    }

    void Audio::setNull3DListener()
    {
        noListener = true;
    }
}