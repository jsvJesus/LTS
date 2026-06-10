#include "Engine/FrameTimer.h"

namespace Engine
{
    FrameTimer::FrameTimer()
    {
        Reset();
    }

    void FrameTimer::Reset()
    {
        mStartTime = Clock::now();
        mPreviousTime = mStartTime;

        mDeltaSeconds = 0.0;
        mTotalSeconds = 0.0;
        mFrameIndex = 0;
    }

    void FrameTimer::Tick()
    {
        const TimePoint currentTime = Clock::now();

        mDeltaSeconds = std::chrono::duration<Core::f64>(currentTime - mPreviousTime).count();
        mTotalSeconds = std::chrono::duration<Core::f64>(currentTime - mStartTime).count();

        mPreviousTime = currentTime;

        ++mFrameIndex;
    }

    Core::f64 FrameTimer::GetDeltaSeconds() const
    {
        return mDeltaSeconds;
    }

    Core::f64 FrameTimer::GetTotalSeconds() const
    {
        return mTotalSeconds;
    }

    Core::u64 FrameTimer::GetFrameIndex() const
    {
        return mFrameIndex;
    }
}