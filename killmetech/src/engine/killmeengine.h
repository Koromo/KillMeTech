#ifndef _KIKKMEENGINE_H_
#define _KILLMEENGINE_H_

#include "../core/string.h"
#include <Windows.h>
#include <memory>
#include <type_traits>

namespace killme
{
    class Console;
    class AudioEngine;
    class EventDispatcher;

    /** KillMeTech runtime */
    class KillMeEngine
    {
    private:
        std::unique_ptr<std::remove_pointer_t<HWND>, decltype(&DestroyWindow)> window_;
        bool quit_;

        // Subsystems
        std::shared_ptr<AudioEngine> audioEngine_;
        std::shared_ptr<EventDispatcher> eventDispatcher_;

    public:
        /** Startup with window parameters */
        KillMeEngine(size_t width, size_t height, const tstring& title);

        /** Shutdown engine */
        ~KillMeEngine();

        /** Start running */
        void run();

        /** Post quit request */
        void quit();

        /** Returns audio engine */
        std::weak_ptr<AudioEngine> getAudioEngine();

        /** Returns event dispatcher */
        std::weak_ptr<EventDispatcher> getEventDispatcher();

    private:
        static LRESULT CALLBACK KillMeEngine::windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp);
    };
}

#endif