#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

#include "Engine.h"
#include "FrameLimiter.h"
#include "FrameStats.h"

#include "Core/BaseTypes.h"

namespace Platform
{
    class Window;
    class InputSystem;
}

namespace Render
{
    class RenderSystem;
}

namespace Engine
{
    class Camera;

    class EngineLoop final
    {
    public:
        EngineLoop();
        ~EngineLoop();

        EngineLoop(const EngineLoop&) = delete;
        EngineLoop& operator=(const EngineLoop&) = delete;

        bool Initialize(const FApplicationDesc& desc);
        int Run();
        void Shutdown();

    private:
        void Tick(double deltaSeconds);
        void RenderFrame(double deltaSeconds);
        void HandleResize();

        void UpdateFrameStats(double deltaSeconds);
        void UpdateWindowDebugTitle();

        void LogFrameLimiterState() const;
        Core::String BuildFrameLimiterLogLine() const;

        void ToggleDebugRendering();
        Core::String BuildDebugRenderingLogLine() const;

        void ToggleCursorLock();
        Core::String BuildInputDebugLogLine() const;

        void InitializeCamera(std::uint32_t width, std::uint32_t height);
        void UpdateCamera(double deltaSeconds);
        void UpdateCameraAspectRatio();
        void ToggleCameraControl();
        Core::String BuildCameraDebugLogLine() const;

    private:
        bool mInitialized = false;
        bool mRunning = false;

        bool mEnableFrameStatsTitle = true;
        double mFrameStatsTitleUpdateIntervalSeconds = 0.5;

        std::uint64_t mFrameIndex = 0;

        Core::String mBaseWindowTitle;

        FrameStats mFrameStats;
        FrameLimiter mFrameLimiter;

        bool mCameraControlEnabled = true;

        std::unique_ptr<Platform::Window> mWindow;
        std::unique_ptr<Platform::InputSystem> mInputSystem;
        std::unique_ptr<Render::RenderSystem> mRenderSystem;
        std::unique_ptr<Camera> mCamera;

        std::chrono::steady_clock::time_point mLastFrameTime {};
    };
}