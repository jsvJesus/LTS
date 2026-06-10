#include "Engine/EngineLoop.h"

#include "Core/Logger.h"

#include <chrono>
#include <string>
#include <thread>

namespace Engine
{
    EngineLoop::~EngineLoop()
    {
        Shutdown();
    }

    bool EngineLoop::Initialize(const EngineCreateInfo& createInfo)
    {
        if (mInitialized)
        {
            Core::Logger::Warning("EngineLoop", "Initialize called, but engine loop is already initialized.");
            return true;
        }

        mCreateInfo = createInfo;

        if (mCreateInfo.ApplicationName.empty())
        {
            mCreateInfo.ApplicationName = "Application";
        }

        if (mCreateInfo.TargetFrameRate == 0)
        {
            mCreateInfo.EnableFrameLimit = false;
        }

        Core::Logger::Info("EngineLoop", "Initializing engine loop.");
        Core::Logger::Info("EngineLoop", "Application: " + mCreateInfo.ApplicationName);

        if (!mMainWindow.Create(mCreateInfo.MainWindow))
        {
            Core::Logger::Fatal("EngineLoop", "Failed to create main window.");
            return false;
        }

        mMainWindow.Show();

        if (mCreateInfo.EnableRendering)
        {
            Render::DX11DeviceCreateInfo renderCreateInfo;
            renderCreateInfo.NativeWindowHandle = mMainWindow.GetNativeHandle();
            renderCreateInfo.Width = mMainWindow.GetWidth();
            renderCreateInfo.Height = mMainWindow.GetHeight();
            renderCreateInfo.EnableVSync = mCreateInfo.EnableVSync;

        #if defined(GAME_DEBUG)
            renderCreateInfo.EnableDebugLayer = true;
        #else
            renderCreateInfo.EnableDebugLayer = false;
        #endif

            if (!mRenderDevice.Initialize(renderCreateInfo))
            {
                Core::Logger::Fatal("EngineLoop", "Failed to initialize render device.");
                mMainWindow.Destroy();
                return false;
            }

            mRenderInitialized = true;
        }

        mFrameTimer.Reset();

        mInitialized = true;
        mRunning = false;
        mShutdownRequested = false;

        Core::Logger::Info("EngineLoop", "Engine loop initialized.");

        return true;
    }

    int EngineLoop::Run()
    {
        if (!mInitialized)
        {
            Core::Logger::Fatal("EngineLoop", "Run called before Initialize.");
            return 1;
        }

        Core::Logger::Info("EngineLoop", "Run started.");

        mRunning = true;

        while (mRunning && !mShutdownRequested)
        {
            if (!mMainWindow.PollEvents())
            {
                RequestShutdown();
                break;
            }

            mFrameTimer.Tick();

            Tick();
            RenderFrame();

            SleepToFrameLimit();
        }

        Core::Logger::Info("EngineLoop", "Run finished.");

        Shutdown();

        return 0;
    }

    void EngineLoop::Shutdown()
    {
        if (!mInitialized)
        {
            return;
        }

        Core::Logger::Info("EngineLoop", "Shutdown started.");

        mRunning = false;
        mShutdownRequested = true;

        if (mRenderInitialized)
        {
            mRenderDevice.Shutdown();
            mRenderInitialized = false;
        }

        mMainWindow.Destroy();

        mInitialized = false;

        Core::Logger::Info("EngineLoop", "Shutdown finished.");
    }

    void EngineLoop::RequestShutdown()
    {
        mShutdownRequested = true;
        mRunning = false;
    }

    bool EngineLoop::IsInitialized() const
    {
        return mInitialized;
    }

    bool EngineLoop::IsRunning() const
    {
        return mRunning;
    }

    bool EngineLoop::IsShutdownRequested() const
    {
        return mShutdownRequested;
    }

    const FrameTimer& EngineLoop::GetFrameTimer() const
    {
        return mFrameTimer;
    }

    Platform::Window& EngineLoop::GetMainWindow()
    {
        return mMainWindow;
    }

    Render::DX11Device& EngineLoop::GetRenderDevice()
    {
        return mRenderDevice;
    }

    void EngineLoop::Tick()
    {
        // Game / Editor update будет здесь.
        // Сейчас главный результат — стабильный loop + render clear.
    }

    void EngineLoop::RenderFrame()
    {
        if (!mRenderInitialized)
        {
            return;
        }

        if (!mRenderDevice.ResizeIfNeeded(
            mMainWindow.GetWidth(),
            mMainWindow.GetHeight()
        ))
        {
            return;
        }

        mRenderDevice.BeginFrame(mCreateInfo.ClearColor);
        mRenderDevice.DrawDebugTriangle();
        mRenderDevice.EndFrame();
    }

    void EngineLoop::SleepToFrameLimit()
    {
        if (!mCreateInfo.EnableFrameLimit || mCreateInfo.TargetFrameRate == 0)
        {
            return;
        }

        const Core::f64 targetFrameSeconds = 1.0 / static_cast<Core::f64>(mCreateInfo.TargetFrameRate);
        const Core::f64 deltaSeconds = mFrameTimer.GetDeltaSeconds();

        if (deltaSeconds >= targetFrameSeconds)
        {
            return;
        }

        const Core::f64 sleepSeconds = targetFrameSeconds - deltaSeconds;

        if (sleepSeconds <= 0.0)
        {
            return;
        }

        const auto sleepDuration = std::chrono::duration<Core::f64>(sleepSeconds);
        std::this_thread::sleep_for(sleepDuration);
    }
}