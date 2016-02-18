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

namespace killme
{
    namespace
    {
        struct RunTimeImpl
        {
            HWND window;
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
    }

    void RunTime::shutdown()
    {
        Debug::shutdown();
        Graphics::shutdown();
        Audio::shutdown();
        Physics::shutdown();
        Events::shutdown();
        Processes::shutdown();
        Resources::shutdown();
        DestroyWindow(impl.window);
    }

    void RunTime::run(LevelDesigner& designer)
    {
        ShowWindow(impl.window, SW_SHOW);

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
                Audio::setNull3DListener();
                Inputs::emitInputEvents();
                designer.tick();
                world->tickActors();
                Physics::tickWorld(1 / 600.0f);
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