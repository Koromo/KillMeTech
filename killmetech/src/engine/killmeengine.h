#ifndef _KILLME_KIKKMEENGINE_H_
#define _KILLME_KILLMEENGINE_H_

#include "../core/string.h"
#include <Windows.h>
#include <memory>
#include <type_traits>

namespace killme
{
    class Console;

    /** KillMeTech runtime */
    class KillMeEngine
    {
    private:
        std::unique_ptr<std::remove_pointer_t<HWND>, decltype(&DestroyWindow)> window_;
        bool quit_;

    public:
        /** Startup with window parameters */
        KillMeEngine(size_t width, size_t height, const tstring& title);

        /** Shutdown engine */
        ~KillMeEngine();

        /** Start running */
        void run();

        /** Post quit request */
        void quit();

    private:
        static LRESULT CALLBACK KillMeEngine::windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp);
    };
}

#endif