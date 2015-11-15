#include "killmeengine.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include "../input/keycode.h"
#include "../input/keyevent.h"
#include "../audio/audioengine.h"
#include "../event/eventdispatcher.h"

namespace killme
{
    KillMeEngine::KillMeEngine(size_t width, size_t height, const tstring& title)
        : window_(nullptr, DestroyWindow)
        , quit_(false)
        , audioEngine_()
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

        // Initialize event system
        eventDispatcher_ = std::make_shared<EventDispatcher>();

        // Initialize audio system
        audioEngine_ = std::make_shared<AudioEngine>();
    }

    KillMeEngine::~KillMeEngine()
    {
        audioEngine_.reset();
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

    namespace
    {
        // Convert WINAPI key code to KillMeTech API key code
        KeyCode toKeyCode(WPARAM vkc)
        {
            switch (vkc)
            {
            case VK_ESCAPE: return KeyCode::Esc;

            case 0x41: return KeyCode::A;
            case 0x42: return KeyCode::B;
            case 0x43: return KeyCode::C;
            case 0x44: return KeyCode::D;
            case 0x45: return KeyCode::E;
            case 0x46: return KeyCode::F;
            case 0x47: return KeyCode::G;
            case 0x48: return KeyCode::H;
            case 0x49: return KeyCode::I;
            case 0x4a: return KeyCode::J;
            case 0x4b: return KeyCode::K;
            case 0x4c: return KeyCode::L;
            case 0x4d: return KeyCode::M;
            case 0x4e: return KeyCode::N;
            case 0x4f: return KeyCode::O;
            case 0x50: return KeyCode::P;
            case 0x51: return KeyCode::Q;
            case 0x52: return KeyCode::R;
            case 0x53: return KeyCode::S;
            case 0x54: return KeyCode::T;
            case 0x55: return KeyCode::U;
            case 0x56: return KeyCode::V;
            case 0x57: return KeyCode::W;
            case 0x58: return KeyCode::X;
            case 0x59: return KeyCode::Y;
            case 0x5a: return KeyCode::Z;

            default:
                return KeyCode::none;
            }
        }
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
            [=]()
            {
                const auto key = toKeyCode(wp);
                if (key != KeyCode::none)
                {
					if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
					{
						engine->eventDispatcher_->dispatch(KeyPressed(key));
					}
					else
					{
						engine->eventDispatcher_->dispatch(KeyReleased(key));
					}
                }
            }();
            break;

        default:
            break;
        }

        return DefWindowProc(window, msg, wp, lp);
    }
}