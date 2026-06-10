#pragma once

#include "Engine/Engine.h"
#include "Engine/FrameTimer.h"

namespace Engine
{
    class EngineLoop final
    {
    public:
        EngineLoop() = default;
        ~EngineLoop();

        EngineLoop(const EngineLoop&) = delete;
        EngineLoop(EngineLoop&&) = delete;

        EngineLoop& operator=(const EngineLoop&) = delete;
        EngineLoop& operator=(EngineLoop&&) = delete;

        bool Initialize(const EngineCreateInfo& createInfo);
        int Run();
        void Shutdown();

        void RequestShutdown();

        [[nodiscard]] bool IsInitialized() const;
        [[nodiscard]] bool IsRunning() const;
        [[nodiscard]] bool IsShutdownRequested() const;

        [[nodiscard]] const FrameTimer& GetFrameTimer() const;
        [[nodiscard]] Platform::Window& GetMainWindow();
        [[nodiscard]] Render::DX11Device& GetRenderDevice();

    private:
        void Tick();
        void RenderFrame();
        void SleepToFrameLimit();

    private:
        EngineCreateInfo mCreateInfo{};

        Platform::Window mMainWindow;
        Render::DX11Device mRenderDevice;
        FrameTimer mFrameTimer;

        bool mInitialized = false;
        bool mRunning = false;
        bool mShutdownRequested = false;
        bool mRenderInitialized = false;
    };
}