#ifndef _KILLME_KIKKMEENGINE_H_
#define _KILLME_KILLMEENGINE_H_

#include "../core/string.h"
#include "../input/keycode.h"
#include <Windows.h>
#include <memory>
#include <type_traits>

namespace killme
{
    class Console;
    class AudioEngine;
    class InputManager;
    class EventDispatcher;
    class SceneManager;

    /** KillMeTech runtime */
    class KillMeEngine
    {
    private:
        std::unique_ptr<std::remove_pointer_t<HWND>, decltype(&DestroyWindow)> window_;
        bool quit_;

        // Subsystems
        std::shared_ptr<AudioEngine> audioEngine_;
        std::shared_ptr<InputManager> inputManager_;
        std::shared_ptr<EventDispatcher> eventDispatcher_;
        std::shared_ptr<SceneManager> sceneManager_;

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
        std::shared_ptr<AudioEngine> getAudioEngine();

        /** Returns event dispatcher */
        std::shared_ptr<EventDispatcher> getEventDispatcher();

        /** Returns scene manager */
        std::shared_ptr<SceneManager> getSceneManager();

    private:
        static LRESULT CALLBACK KillMeEngine::windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp);
    };
}

#endif