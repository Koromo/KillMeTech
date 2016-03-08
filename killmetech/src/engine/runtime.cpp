#include "runtime.h"
#include "level.h"
#include "audiosystem.h"
#include "graphicssystem.h"
#include "resourcemanagesystem.h"
#include "inputmanager.h"
#include "debug.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <cassert>

namespace killme
{
    RunTime runTime;

    namespace
    {
        LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
        {
            switch (msg)
            {
            case WM_CLOSE:
                PostQuitMessage(0);
                break;

            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
                inputManager.onWinKeyDown(wp);
                break;

            case WM_SYSKEYUP:
            case WM_KEYUP:
                inputManager.onWinKeyUp(wp);
                break;

            default:
                break;
            }

            return DefWindowProc(window, msg, wp, lp);
        }
    }

    RunTime::RunTime()
        : window_(nullptr, DestroyWindow)
        , frameTimes_ms_()
        , frameCounter_()
        , preFrameDate_()
        , currentDeltaTimes_s_()
    {
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
        window_.reset(enforce<WindowsException>(
            CreateWindow(wc.lpszClassName, wc.lpszClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
                windowWidth, windowHeight, NULL, NULL, instance, NULL),
            "Failed to create the window."));

        // Initialize subsystems
        resourceManager.startup();
        audioSystem.startup();
        graphicsSystem.startup(window_.get());
        KILLME_DEBUG_INITIALIZE();

        // Set to 60fps
        setFrameRate(FrameRate::_60);
        frameCounter_ = 0;
        preFrameDate_ = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < NUM_STORE_DELTATIMES; ++i)
        {
            currentDeltaTimes_s_.push_back(1 / 60.0f);
        }
    }

    void RunTime::shutdown()
    {
        currentDeltaTimes_s_.clear();
        KILLME_DEBUG_FINALIZE();
        graphicsSystem.shutdown();
        audioSystem.shutdown();
        resourceManager.shutdown();
    }

    void RunTime::setFrameRate(FrameRate fps)
    {
        switch (fps)
        {
        case FrameRate::noLimit:
            frameTimes_ms_[0] = 0;
            frameTimes_ms_[1] = 0;
            frameTimes_ms_[2] = 0;
            break;

        case FrameRate::_120:
            frameTimes_ms_[0] = 8;
            frameTimes_ms_[1] = 8;
            frameTimes_ms_[2] = 9;
            break;

        case FrameRate::_60:
            frameTimes_ms_[0] = 16;
            frameTimes_ms_[1] = 17;
            frameTimes_ms_[2] = 17;
            break;

        case FrameRate::_30:
            frameTimes_ms_[0] = 30;
            frameTimes_ms_[1] = 30;
            frameTimes_ms_[2] = 40;
            break;

        case FrameRate::_20:
            frameTimes_ms_[0] = 50;
            frameTimes_ms_[1] = 50;
            frameTimes_ms_[2] = 50;
            break;

        case FrameRate::_15:
            frameTimes_ms_[0] = 60;
            frameTimes_ms_[1] = 70;
            frameTimes_ms_[2] = 70;
            break;

        default:
            assert(false && "Not supportted fps.");
            break;
        }
    }

    float RunTime::getDeltaTime() const
    {
        return currentDeltaTimes_s_.back();
    }

    float RunTime::getCurrentFrameRate(size_t n) const
    {
        if (n > NUM_STORE_DELTATIMES)
        {
            n = NUM_STORE_DELTATIMES;
        }

        float sum = 0;
        for (size_t i = 0; i < n; ++i)
        {
            sum += currentDeltaTimes_s_[i];
        }
        return 1 / (sum / n);
    }

    void RunTime::run(Level& level)
    {
        ShowWindow(window_.get(), SW_SHOW);

        // Build level
        level.beginLevel();

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
                auto dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentDate - preFrameDate_).count();
                if (dt_ms < frameTimes_ms_[frameCounter_])
                {
                    const auto sleepTime = frameTimes_ms_[frameCounter_] - dt_ms;
                    Sleep(static_cast<DWORD>(sleepTime));
                    currentDate = std::chrono::high_resolution_clock::now();
                    dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentDate - preFrameDate_).count();
                }
                frameCounter_;
                frameCounter_ %= FRAME_CYCLE;
                preFrameDate_ = currentDate;

                auto dt_s = dt_ms * 0.001f;
                currentDeltaTimes_s_.pop_front();
                currentDeltaTimes_s_.push_back(dt_s);
                assert(currentDeltaTimes_s_.size() == NUM_STORE_DELTATIMES && "Delta time queue error.");

                // Tick frame
                level.beginFrame();
                inputManager.emitInputEvents(level);
                level.tickLevel(dt_s);
                graphicsSystem.clearBackBuffer();
                level.renderLevel(graphicsSystem.getCurrentFrameResource());
                KILLME_DEBUG_DRAW(level.getGraphicsWorld(), graphicsSystem.getCurrentFrameResource());
                graphicsSystem.presentBackBuffer();
            }
        }

        level.endLevel();
    }

    void RunTime::quit()
    {
        // Request to close window
        PostMessage(window_.get(), WM_CLOSE, 0, 0);
    }
}