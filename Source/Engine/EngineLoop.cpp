#include "EngineLoop.h"

#include "../Core/Logger.h"
#include "../Platform/Input.h"
#include "../Platform/Window.h"
#include "../Render/RenderSystem.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <iomanip>
#include <sstream>

namespace Engine
{
    namespace
    {
        Core::String WideToUtf8(const wchar_t* value)
        {
            if (value == nullptr || value[0] == L'\0')
                return "Application";

            const int requiredSize = ::WideCharToMultiByte(
                CP_UTF8,
                0,
                value,
                -1,
                nullptr,
                0,
                nullptr,
                nullptr
            );

            if (requiredSize <= 0)
                return "Application";

            Core::String result;
            result.resize(static_cast<std::size_t>(requiredSize - 1));

            ::WideCharToMultiByte(
                CP_UTF8,
                0,
                value,
                -1,
                result.data(),
                requiredSize,
                nullptr,
                nullptr
            );

            return result;
        }

        Render::FRenderColor ToRenderColor(const FApplicationColor& color)
        {
            Render::FRenderColor result {};
            result.R = color.R;
            result.G = color.G;
            result.B = color.B;
            result.A = color.A;
            return result;
        }

        Core::String BuildDebugTitle(
            const Core::String& baseTitle,
            const Core::i32 width,
            const Core::i32 height,
            const FFrameStatsSnapshot& stats,
            const FFrameLimiterSnapshot& limiter,
            const bool debugRenderingEnabled
        )
        {
            std::ostringstream stream;

            stream << baseTitle
                   << " | "
                   << Core::GetBuildConfigurationName()
                   << " | "
                   << width
                   << "x"
                   << height
                   << " | FPS: "
                   << std::fixed
                   << std::setprecision(1)
                   << stats.FramesPerSecond
                   << " | Frame: "
                   << std::fixed
                   << std::setprecision(2)
                   << stats.AverageFrameMilliseconds
                   << " ms";

            if (limiter.Enabled)
            {
                stream << " | Limit: "
                       << std::fixed
                       << std::setprecision(0)
                       << limiter.TargetFrameRate;
            }
            else
            {
                stream << " | Limit: Off";
            }

            stream << " | DebugDraw: "
                   << (debugRenderingEnabled ? "On" : "Off");

            return stream.str();
        }

        Core::String BuildFrameStatsLogLine(const FFrameStatsSnapshot& stats)
        {
            std::ostringstream stream;

            stream << "FrameStats: "
                   << "FrameIndex="
                   << stats.FrameIndex
                   << ", FPS="
                   << std::fixed
                   << std::setprecision(1)
                   << stats.FramesPerSecond
                   << ", AverageFrameMs="
                   << std::fixed
                   << std::setprecision(2)
                   << stats.AverageFrameMilliseconds
                   << ", LastFrameMs="
                   << std::fixed
                   << std::setprecision(2)
                   << stats.LastFrameMilliseconds
                   << ", FramesInSample="
                   << stats.FramesInSample;

            return stream.str();
        }
    }

    EngineLoop::EngineLoop() = default;

    EngineLoop::~EngineLoop()
    {
        Shutdown();
    }

    bool EngineLoop::Initialize(const FApplicationDesc& desc)
    {
        Shutdown();

        mBaseWindowTitle = WideToUtf8(desc.Title);
        mEnableFrameStatsTitle = desc.EnableFrameStatsTitle;
        mFrameStatsTitleUpdateIntervalSeconds = desc.FrameStatsTitleUpdateIntervalSeconds > 0.01
            ? desc.FrameStatsTitleUpdateIntervalSeconds
            : 0.5;

        mFrameStats.Reset();
        mFrameLimiter.Configure(desc.EnableFrameLimit, desc.TargetFrameRate);

        mWindow = std::make_unique<Platform::Window>();

        Platform::WindowCreateInfo windowInfo {};
        windowInfo.Title = mBaseWindowTitle;
        windowInfo.Width = static_cast<Core::i32>(desc.Width);
        windowInfo.Height = static_cast<Core::i32>(desc.Height);
        windowInfo.StartCentered = true;
        windowInfo.Resizable = true;
        windowInfo.VisibleOnCreate = true;

        if (!mWindow->Create(windowInfo))
        {
            Shutdown();
            return false;
        }

        mInputSystem = std::make_unique<Platform::InputSystem>();

        if (!mInputSystem->Initialize(mWindow->GetNativeHandle()))
        {
            Shutdown();
            return false;
        }

        Render::FRenderSystemDesc renderDesc {};
        renderDesc.NativeWindowHandle = mWindow->GetNativeHandle();
        renderDesc.Width = static_cast<std::uint32_t>(mWindow->GetWidth());
        renderDesc.Height = static_cast<std::uint32_t>(mWindow->GetHeight());

#if defined(_DEBUG)
        renderDesc.EnableDebugLayer = true;
#else
        renderDesc.EnableDebugLayer = false;
#endif

        renderDesc.EnableDebugRenderer = desc.EnableDebugRenderer;
        renderDesc.EnableDebugRendering = desc.EnableDebugRendering;
        renderDesc.EnableVSync = desc.EnableVSync;
        renderDesc.ClearColor = ToRenderColor(desc.ClearColor);

        mRenderSystem = std::make_unique<Render::RenderSystem>();

        if (!mRenderSystem->Initialize(renderDesc))
        {
            Shutdown();
            return false;
        }

        mLastFrameTime = std::chrono::steady_clock::now();

        mFrameIndex = 0;
        mRunning = true;
        mInitialized = true;

        UpdateWindowDebugTitle();

        Core::Logger::Info("Engine", "Engine loop initialized.");
        LogFrameLimiterState();
        Core::Logger::Info("Engine", BuildDebugRenderingLogLine());

        return true;
    }

    int EngineLoop::Run()
    {
        if (!mInitialized)
            return -1;

        while (mRunning)
        {
            const auto frameStartTime = std::chrono::steady_clock::now();

            if (!mWindow || !mWindow->PollEvents())
            {
                mRunning = false;
                break;
            }

            if (mWindow->IsCloseRequested() || !mWindow->IsOpen())
            {
                mRunning = false;
                break;
            }

            const std::chrono::duration<double> delta =
                frameStartTime - mLastFrameTime;

            mLastFrameTime = frameStartTime;

            const double deltaSeconds = delta.count();

            HandleResize();

            if (mInputSystem)
            {
                mInputSystem->Update();
            }

            Tick(deltaSeconds);
            RenderFrame(deltaSeconds);
            UpdateFrameStats(deltaSeconds);

            ++mFrameIndex;

            mFrameLimiter.WaitIfNeeded(frameStartTime);
        }

        return 0;
    }

    void EngineLoop::Shutdown()
    {
        mRunning = false;

        if (mRenderSystem)
        {
            mRenderSystem->Shutdown();
            mRenderSystem.reset();
        }

        if (mInputSystem)
        {
            mInputSystem->Shutdown();
            mInputSystem.reset();
        }

        if (mWindow)
        {
            mWindow->Destroy();
            mWindow.reset();
        }

        mFrameStats.Reset();
        mFrameLimiter.Reset();

        mBaseWindowTitle.clear();

        mFrameIndex = 0;
        mInitialized = false;
    }

    void EngineLoop::Tick(double deltaSeconds)
    {
        (void)deltaSeconds;

        if (mInputSystem && mInputSystem->IsKeyPressed(Platform::KeyCode::F1))
        {
            Core::Logger::Info("Input", "F1 pressed.");
        }

        if (mInputSystem && mInputSystem->IsKeyPressed(Platform::KeyCode::F2))
        {
            Core::Logger::Info("Engine", BuildFrameStatsLogLine(mFrameStats.GetSnapshot()));
            Core::Logger::Info("Engine", BuildFrameLimiterLogLine());
            Core::Logger::Info("Engine", BuildDebugRenderingLogLine());
        }

        if (mInputSystem && mInputSystem->IsKeyPressed(Platform::KeyCode::F3))
        {
            mFrameLimiter.ToggleEnabled();
            LogFrameLimiterState();
            UpdateWindowDebugTitle();
        }

        if (mInputSystem && mInputSystem->IsKeyPressed(Platform::KeyCode::F4))
        {
            ToggleDebugRendering();
        }

        if (mInputSystem && mInputSystem->IsMouseButtonPressed(Platform::MouseButton::Left))
        {
            Core::Logger::Info("Input", "Left mouse button pressed.");
        }

        // Позже тут будут:
        // input mapping
        // editor camera
        // game camera
        // world tick
        // editor tick
        // game tick
    }

    void EngineLoop::RenderFrame(double deltaSeconds)
    {
        if (!mRenderSystem)
            return;

        Render::FRenderFrameInfo frameInfo {};
        frameInfo.DeltaSeconds = deltaSeconds;
        frameInfo.FrameIndex = mFrameIndex;

        mRenderSystem->BeginFrame(frameInfo);

        mRenderSystem->RenderDebug();

        mRenderSystem->EndFrame();
    }

    void EngineLoop::HandleResize()
    {
        if (!mWindow || !mRenderSystem)
            return;

        const Core::i32 windowWidth = mWindow->GetWidth();
        const Core::i32 windowHeight = mWindow->GetHeight();

        if (windowWidth <= 0 || windowHeight <= 0)
            return;

        const std::uint32_t width = static_cast<std::uint32_t>(windowWidth);
        const std::uint32_t height = static_cast<std::uint32_t>(windowHeight);

        if (width == mRenderSystem->GetWidth() && height == mRenderSystem->GetHeight())
            return;

        mRenderSystem->Resize(width, height);
        UpdateWindowDebugTitle();
    }

    void EngineLoop::UpdateFrameStats(const double deltaSeconds)
    {
        const bool shouldUpdateTitle = mFrameStats.Update(
            deltaSeconds,
            mFrameIndex,
            mFrameStatsTitleUpdateIntervalSeconds
        );

        if (shouldUpdateTitle)
        {
            UpdateWindowDebugTitle();
        }
    }

    void EngineLoop::UpdateWindowDebugTitle()
    {
        if (!mEnableFrameStatsTitle || !mWindow)
            return;

        const bool debugRenderingEnabled =
            mRenderSystem && mRenderSystem->IsDebugRenderingEnabled();

        const Core::String title = BuildDebugTitle(
            mBaseWindowTitle,
            mWindow->GetWidth(),
            mWindow->GetHeight(),
            mFrameStats.GetSnapshot(),
            mFrameLimiter.GetSnapshot(),
            debugRenderingEnabled
        );

        mWindow->SetTitle(title);
    }

    void EngineLoop::LogFrameLimiterState() const
    {
        Core::Logger::Info("Engine", BuildFrameLimiterLogLine());
    }

    Core::String EngineLoop::BuildFrameLimiterLogLine() const
    {
        const FFrameLimiterSnapshot& limiter = mFrameLimiter.GetSnapshot();

        std::ostringstream stream;

        stream << "FrameLimiter: "
               << "Enabled="
               << (limiter.Enabled ? "true" : "false")
               << ", TargetFPS="
               << std::fixed
               << std::setprecision(1)
               << limiter.TargetFrameRate
               << ", TargetFrameMs="
               << std::fixed
               << std::setprecision(2)
               << limiter.TargetFrameMilliseconds
               << ", LastSleepMs="
               << std::fixed
               << std::setprecision(2)
               << limiter.LastSleepMilliseconds;

        return stream.str();
    }

    void EngineLoop::ToggleDebugRendering()
    {
        if (!mRenderSystem)
            return;

        if (!mRenderSystem->IsDebugRendererAvailable())
        {
            Core::Logger::Warning("Engine", "Debug rendering toggle ignored. Debug renderer is not available.");
            return;
        }

        mRenderSystem->ToggleDebugRendering();

        Core::Logger::Info("Engine", BuildDebugRenderingLogLine());

        UpdateWindowDebugTitle();
    }

    Core::String EngineLoop::BuildDebugRenderingLogLine() const
    {
        const bool available = mRenderSystem && mRenderSystem->IsDebugRendererAvailable();
        const bool enabled = mRenderSystem && mRenderSystem->IsDebugRenderingEnabled();

        std::ostringstream stream;

        stream << "DebugRendering: "
               << "Available="
               << (available ? "true" : "false")
               << ", Enabled="
               << (enabled ? "true" : "false");

        return stream.str();
    }
}