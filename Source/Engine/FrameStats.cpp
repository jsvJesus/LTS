#include "FrameStats.h"

namespace Engine
{
    void FrameStats::Reset()
    {
        mSnapshot = {};
        mAccumulatedSeconds = 0.0;
        mAccumulatedFrames = 0;
    }

    bool FrameStats::Update(
        const double deltaSeconds,
        const Core::u64 frameIndex,
        const double sampleIntervalSeconds
    )
    {
        const double safeDeltaSeconds = deltaSeconds > 0.0 ? deltaSeconds : 0.0;
        const double safeSampleIntervalSeconds =
            sampleIntervalSeconds > 0.01 ? sampleIntervalSeconds : 0.5;

        mSnapshot.FrameIndex = frameIndex;
        mSnapshot.LastDeltaSeconds = safeDeltaSeconds;
        mSnapshot.LastFrameMilliseconds = safeDeltaSeconds * 1000.0;

        mAccumulatedSeconds += safeDeltaSeconds;
        ++mAccumulatedFrames;

        if (mAccumulatedSeconds < safeSampleIntervalSeconds)
        {
            return false;
        }

        mSnapshot.FramesInSample = mAccumulatedFrames;
        mSnapshot.AverageDeltaSeconds =
            mAccumulatedFrames > 0
                ? mAccumulatedSeconds / static_cast<double>(mAccumulatedFrames)
                : 0.0;

        mSnapshot.AverageFrameMilliseconds = mSnapshot.AverageDeltaSeconds * 1000.0;

        mSnapshot.FramesPerSecond =
            mAccumulatedSeconds > 0.0
                ? static_cast<double>(mAccumulatedFrames) / mAccumulatedSeconds
                : 0.0;

        mAccumulatedSeconds = 0.0;
        mAccumulatedFrames = 0;

        return true;
    }
}