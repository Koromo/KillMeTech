#ifndef _KILLME_RUNTIME_H_
#define _KILLME_RUNTIME_H_

#include "../core/string.h"
#include <Windows.h>
#include <type_traits>
#include <chrono>
#include <deque>
#include <array>
#include <utility>
#include <memory>

namespace killme
{
    class Level;

    /** Frame rate definition */
    enum class FrameRate : size_t
    {
        noLimit,
        _120,
        _60,
        _30,
        _20,
        _15
    };

    /** KillMe Tech runtime engine */
    class RunTime
    {
    private:
        static constexpr size_t FRAME_CYCLE = 3;
        static constexpr size_t NUM_STORE_DELTATIMES = 120;

        std::unique_ptr<std::remove_pointer_t<HWND>, decltype(&DestroyWindow)> window_;
        std::array<long long, FRAME_CYCLE> frameTimes_ms_;
        size_t frameCounter_;
        decltype(std::chrono::high_resolution_clock::now()) preFrameDate_;
        std::deque<float> currentDeltaTimes_s_;

    public:
        /** Constructs */
        RunTime();

        /** Initialize KillMe Tech */
        void startup(size_t width, size_t height, const tstring& title);

        /** Finalize KillMe Tech */
        void shutdown();

        /** Set FPS */
        void setFrameRate(FrameRate fps);

        /** Return the delta time[s] of current frame from previous */
        float getDeltaTime() const;

        /** Return the average fps of current n frames */
        /// NOTE: n <= 120
        float getCurrentFrameRate(size_t n) const;

        // For newLevel()
        void run(Level& level);

        /** Request to exit the game */
        void quit();
    };

    extern RunTime runTime;

    /** Start new level */
    template <class T, class... Args>
    void newLevel(Args&&... args)
    {
        T level(std::forward<Args>(args)...);
        runTime.run(level);
    }
}

#endif