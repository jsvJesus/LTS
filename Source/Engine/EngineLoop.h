#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

#include "Engine.h"

namespace Platform
{
    class Window;
}

namespace Render
{
    class RenderSystem;
}

namespace Engine
{
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

    private:
        bool mInitialized = false;
        bool mRunning = false;

        std::uint64_t mFrameIndex = 0;

        std::unique_ptr<Platform::Window> mWindow;
        std::unique_ptr<Render::RenderSystem> mRenderSystem;

        std::chrono::steady_clock::time_point mLastFrameTime {};
    };
}