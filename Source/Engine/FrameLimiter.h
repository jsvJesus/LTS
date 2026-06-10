#pragma once

#include "Core/BaseTypes.h"

#include <chrono>

namespace Engine
{
    struct FFrameLimiterSnapshot final
    {
        bool Enabled = false;

        double TargetFrameRate = 0.0;
        double TargetFrameSeconds = 0.0;
        double TargetFrameMilliseconds = 0.0;

        double LastSleepSeconds = 0.0;
        double LastSleepMilliseconds = 0.0;
    };

    class FrameLimiter final
    {
    public:
        FrameLimiter() = default;

        void Configure(bool enabled, double targetFrameRate);
        void Reset();

        void SetEnabled(bool enabled);
        void ToggleEnabled();

        [[nodiscard]] bool IsEnabled() const
        {
            return mEnabled;
        }

        [[nodiscard]] double GetTargetFrameRate() const
        {
            return mTargetFrameRate;
        }

        [[nodiscard]] const FFrameLimiterSnapshot& GetSnapshot() const
        {
            return mSnapshot;
        }

        void WaitIfNeeded(const std::chrono::steady_clock::time_point& frameStartTime);

    private:
        void RebuildSnapshot();

    private:
        bool mEnabled = false;

        double mTargetFrameRate = 60.0;
        double mTargetFrameSeconds = 1.0 / 60.0;

        FFrameLimiterSnapshot mSnapshot {};
    };
}