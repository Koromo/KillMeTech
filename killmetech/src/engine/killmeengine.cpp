#include "killmeengine.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include "../audio/audioengine.h"
#include "../input/inputmanager.h"
#include "../event/eventdispatcher.h"

namespace killme
{
    KillMeEngine::KillMeEngine(size_t width, size_t height, const tstring& title)
        : window_(nullptr, DestroyWindow)
        , quit_(false)
        , audioEngine_()
        , inputManager_()
        , eventDispatcher_()
    {
        // Initialize window
        // Register window class
        const auto instance = GetModuleHandle(NULL);

        WNDCLASSEX wc;
        wc.cbSize = sizeof(wc);
        wc.style = 0;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = instance;
        wc.lpfnWndProc = KillMeEngine::windowProc;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hIconSm = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        wc.lpszMenuName = NULL;
        wc.lpszClassName = title.c_str();

        enforce<WindowsException>(
            RegisterClassEx(&wc),
            "Failed to register window."
            );

        // Calculate window size
        const auto style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
        RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};

        AdjustWindowRect(&rect, style, false);

        const auto windowWidth = rect.right - rect.left;
        const auto windowHeight = rect.bottom - rect.top;

        // Create window
        const auto window = enforce<WindowsException>(
            CreateWindow(wc.lpszClassName, wc.lpszClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
            windowWidth, windowHeight, NULL, NULL, instance, NULL),
            "Failed to create window."
            );

        window_.reset(window);

        // Initialize audio system
        audioEngine_ = std::make_shared<AudioEngine>();

        // Initialize input system
        inputManager_ = std::make_shared<InputManager>();

        // Initialize event system
        eventDispatcher_ = std::make_shared<EventDispatcher>();
    }

    KillMeEngine::~KillMeEngine()
    {
        audioEngine_.reset();
        inputManager_.reset();
        eventDispatcher_.reset();
        window_.reset();
    }

    void KillMeEngine::run()
    {
        SetWindowLongPtr(window_.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        ShowWindow(window_.get(), SW_SHOW);

        // Game loop
        // WM_QUIT message is sended by WM_CLOSE label in window procedure only
        /** TODO: We want to continue running when after close console.
         *  But application is exit when console is closed by close button. If Use FreeConsole(), application is not exit.
         */
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        SetWindowLongPtr(window_.get(), GWLP_USERDATA, 0);
    }

    void KillMeEngine::quit()
    {
        // Close window
        PostMessage(window_.get(), WM_CLOSE, 0, 0);
    }

    std::weak_ptr<AudioEngine> KillMeEngine::getAudioEngine()
    {
        return audioEngine_;
    }

    std::weak_ptr<EventDispatcher> KillMeEngine::getEventDispatcher()
    {
        return eventDispatcher_;
    }

    LRESULT CALLBACK KillMeEngine::windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
    {
        const auto engine = reinterpret_cast<KillMeEngine*>(GetWindowLongPtr(window, GWLP_USERDATA));
        if (!engine)
        {
            return DefWindowProc(window, msg, wp, lp);
        }

        switch (msg)
        {
        case WM_CLOSE:
            // Post WM_QUIT message
            PostQuitMessage(0);
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            engine->inputManager_->onWinKeyEvent(*(engine->eventDispatcher_), msg, wp);

        default:
            break;
        }

        return DefWindowProc(window, msg, wp, lp);
    }
}