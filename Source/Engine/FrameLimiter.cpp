#include "FrameLimiter.h"

#include <thread>

namespace Engine
{
    namespace
    {
        constexpr double MinTargetFrameRate = 15.0;
        constexpr double MaxTargetFrameRate = 1000.0;

        [[nodiscard]] double ClampTargetFrameRate(const double value)
        {
            if (value < MinTargetFrameRate)
                return MinTargetFrameRate;

            if (value > MaxTargetFrameRate)
                return MaxTargetFrameRate;

            return value;
        }
    }

    void FrameLimiter::Configure(const bool enabled, const double targetFrameRate)
    {
        mEnabled = enabled;
        mTargetFrameRate = ClampTargetFrameRate(targetFrameRate);
        mTargetFrameSeconds = 1.0 / mTargetFrameRate;

        mSnapshot.LastSleepSeconds = 0.0;
        mSnapshot.LastSleepMilliseconds = 0.0;

        RebuildSnapshot();
    }

    void FrameLimiter::Reset()
    {
        mEnabled = false;
        mTargetFrameRate = 60.0;
        mTargetFrameSeconds = 1.0 / 60.0;

        mSnapshot = {};
        RebuildSnapshot();
    }

    void FrameLimiter::SetEnabled(const bool enabled)
    {
        mEnabled = enabled;

        mSnapshot.LastSleepSeconds = 0.0;
        mSnapshot.LastSleepMilliseconds = 0.0;

        RebuildSnapshot();
    }

    void FrameLimiter::ToggleEnabled()
    {
        SetEnabled(!mEnabled);
    }

    void FrameLimiter::WaitIfNeeded(const std::chrono::steady_clock::time_point& frameStartTime)
    {
        mSnapshot.LastSleepSeconds = 0.0;
        mSnapshot.LastSleepMilliseconds = 0.0;

        if (!mEnabled)
        {
            RebuildSnapshot();
            return;
        }

        const auto currentTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed = currentTime - frameStartTime;

        const double remainingSeconds = mTargetFrameSeconds - elapsed.count();

        if (remainingSeconds <= 0.0)
        {
            RebuildSnapshot();
            return;
        }

        const auto sleepDuration = std::chrono::duration<double>(remainingSeconds);
        std::this_thread::sleep_for(sleepDuration);

        mSnapshot.LastSleepSeconds = remainingSeconds;
        mSnapshot.LastSleepMilliseconds = remainingSeconds * 1000.0;

        RebuildSnapshot();
    }

    void FrameLimiter::RebuildSnapshot()
    {
        mSnapshot.Enabled = mEnabled;
        mSnapshot.TargetFrameRate = mTargetFrameRate;
        mSnapshot.TargetFrameSeconds = mTargetFrameSeconds;
        mSnapshot.TargetFrameMilliseconds = mTargetFrameSeconds * 1000.0;
    }
}