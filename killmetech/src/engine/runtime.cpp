#include "runtime.h"
#include "debug.h"
#include "leveldesigner.h"
#include "world.h"
#include "inputs.h"
#include "audio.h"
#include "graphics.h"
#include "physics.h"
#include "events.h"
#include "processes.h"
#include "resources.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <Windows.h>
#include <array>
#include <deque>
#include <chrono>
#include <climits>
#include <cassert>

namespace killme
{
    namespace
    {
        struct RunTimeImpl
        {
            HWND window;

            static constexpr size_t FRAME_CYCLE = 3;
            static constexpr size_t NUM_STORE_DELTATIMES = 120;

            std::array<long long, FRAME_CYCLE> frameTimes_ms;
            size_t frameCounter;
            decltype(std::chrono::high_resolution_clock::now()) preFrameDate;
            std::deque<float> currentDeltaTimes_s;
        };

        RunTimeImpl impl;

        LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
        {
            switch (msg)
            {
            case WM_CLOSE:
                PostQuitMessage(0);
                break;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                Inputs::onWinKeyUp(wp);
                break;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP: {
                Inputs::onWinKeyDown(wp);
                break;
            }

            default:
                break;
            }

            return DefWindowProc(window, msg, wp, lp);
        }
    }

    void RunTime::startup(size_t width, size_t height, const tstring& title)
    {
        // Register the window class
        const auto instance = GetModuleHandle(NULL);

        WNDCLASSEX wc;
        wc.cbSize = sizeof(wc);
        wc.style = 0;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = instance;
        wc.lpfnWndProc = windowProc;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hIconSm = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        wc.lpszMenuName = NULL;
        wc.lpszClassName = title.c_str();

        enforce<WindowsException>(
            RegisterClassEx(&wc),
            "Failed to register the window class.");

        // Calculate window size
        const auto style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
        RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

        AdjustWindowRect(&rect, style, false);

        const auto windowWidth = rect.right - rect.left;
        const auto windowHeight = rect.bottom - rect.top;

        // Create window
        impl.window = enforce<WindowsException>(
            CreateWindow(wc.lpszClassName, wc.lpszClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
                windowWidth, windowHeight, NULL, NULL, instance, NULL),
            "Failed to create the window.");

        // Initialize subsystems
        Resources::startup();
        Processes::startup();
        Events::startup();
        Physics::startup();
        Audio::startup();
        Graphics::startup(impl.window);
        Debug::startup();

        // Set to 60fps
        setFrameRate(FrameRate::_60);
        impl.frameCounter = 0;
        impl.preFrameDate = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < impl.NUM_STORE_DELTATIMES; ++i)
        {
            impl.currentDeltaTimes_s.push_back(1 / 60.0f);
        }
    }

    void RunTime::shutdown()
    {
        impl.currentDeltaTimes_s.clear();
        Debug::shutdown();
        Graphics::shutdown();
        Audio::shutdown();
        Physics::shutdown();
        Events::shutdown();
        Processes::shutdown();
        Resources::shutdown();
        DestroyWindow(impl.window);
    }

    void RunTime::setFrameRate(FrameRate fps)
    {
        switch (fps)
        {
        case FrameRate::noLimit:
            impl.frameTimes_ms[0] = 0;
            impl.frameTimes_ms[1] = 0;
            impl.frameTimes_ms[2] = 0;
            break;

        case FrameRate::_60:
            impl.frameTimes_ms[0] = 16;
            impl.frameTimes_ms[1] = 17;
            impl.frameTimes_ms[2] = 17;
            break;

        case FrameRate::_30:
            impl.frameTimes_ms[0] = 30;
            impl.frameTimes_ms[1] = 30;
            impl.frameTimes_ms[2] = 40;
            break;

        case FrameRate::_20:
            impl.frameTimes_ms[0] = 50;
            impl.frameTimes_ms[1] = 50;
            impl.frameTimes_ms[2] = 50;
            break;

        case FrameRate::_15:
            impl.frameTimes_ms[0] = 60;
            impl.frameTimes_ms[1] = 70;
            impl.frameTimes_ms[2] = 70;
            break;

        default:
            assert(false && "Not supportted fps.");
            break;
        }
    }

    float RunTime::getDeltaTime()
    {
        return impl.currentDeltaTimes_s.back();
    }

    float RunTime::getCurrentFrameRate(size_t n)
    {
        if (n > impl.NUM_STORE_DELTATIMES)
        {
            n = impl.NUM_STORE_DELTATIMES;
        }

        float sum = 0;
        for (size_t i = 0; i < n; ++i)
        {
            sum += impl.currentDeltaTimes_s[i];
        }
        return 1 / (sum / n);
    }

    void RunTime::run(LevelDesigner& designer)
    {
        ShowWindow(impl.window, SW_SHOW);

        // Build world
        const auto world = std::make_shared<World>();
        designer.setWorld(world);
        designer.build();

        // Game loop until receive WM_QUIT. WM_QUIT is only sended by process WM_CLOSE
        /// TODO: If the console is allocated, then we want to continue the game loop when console is closed.
        ///       However, WM_QUIT is seceived on closed the console.
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // Adjust FPS
                auto currentDate = std::chrono::high_resolution_clock::now();
                auto dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentDate - impl.preFrameDate).count();
                if (dt_ms < impl.frameTimes_ms[impl.frameCounter])
                {
                    const auto sleepTime = impl.frameTimes_ms[impl.frameCounter] - dt_ms;
                    Sleep(static_cast<DWORD>(sleepTime));
                    currentDate = std::chrono::high_resolution_clock::now();
                    dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentDate - impl.preFrameDate).count();
                }
                ++impl.frameCounter;
                impl.frameCounter %= impl.FRAME_CYCLE;
                impl.preFrameDate = currentDate;

                auto dt_s = dt_ms * 0.001f;
                impl.currentDeltaTimes_s.pop_front();
                impl.currentDeltaTimes_s.push_back(dt_s);
                assert(impl.currentDeltaTimes_s.size() == impl.NUM_STORE_DELTATIMES && "Delta time queue error.");

                // Tick frame
                Audio::setNull3DListener();
                Inputs::emitInputEvents();
                designer.tick(dt_s);
                world->tickActors(dt_s);
                Physics::tickWorld(dt_s);
                Processes::update(0, PROCESS_PRIORITY_MAX);
                Graphics::renderScene();
                Debug::debugDraw();
                Graphics::presentBackBuffer();
            }
        }

        designer.setWorld(nullptr);
    }

    void RunTime::run(LevelDesigner&& designer)
    {
        RunTime::run(designer);
    }

    void RunTime::quit()
    {
        // Request to close window
        PostMessage(impl.window, WM_CLOSE, 0, 0);
    }
}