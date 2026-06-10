#pragma once

#include "Core/BaseTypes.h"

namespace Engine
{
    struct FFrameStatsSnapshot final
    {
        Core::u64 FrameIndex = 0;

        double LastDeltaSeconds = 0.0;
        double LastFrameMilliseconds = 0.0;

        double AverageDeltaSeconds = 0.0;
        double AverageFrameMilliseconds = 0.0;

        double FramesPerSecond = 0.0;

        Core::u32 FramesInSample = 0;
    };

    class FrameStats final
    {
    public:
        FrameStats() = default;

        void Reset();

        bool Update(
            double deltaSeconds,
            Core::u64 frameIndex,
            double sampleIntervalSeconds
        );

        [[nodiscard]] const FFrameStatsSnapshot& GetSnapshot() const
        {
            return mSnapshot;
        }

    private:
        FFrameStatsSnapshot mSnapshot {};

        double mAccumulatedSeconds = 0.0;
        Core::u32 mAccumulatedFrames = 0;
    };
}