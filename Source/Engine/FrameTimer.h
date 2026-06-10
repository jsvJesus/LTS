#pragma once

#include "Core/BaseTypes.h"

#include <chrono>

namespace Engine
{
    class FrameTimer final
    {
    public:
        FrameTimer();

        void Reset();
        void Tick();

        [[nodiscard]] Core::f64 GetDeltaSeconds() const;
        [[nodiscard]] Core::f64 GetTotalSeconds() const;
        [[nodiscard]] Core::u64 GetFrameIndex() const;

    private:
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;

        TimePoint mStartTime{};
        TimePoint mPreviousTime{};

        Core::f64 mDeltaSeconds = 0.0;
        Core::f64 mTotalSeconds = 0.0;

        Core::u64 mFrameIndex = 0;
    };
}