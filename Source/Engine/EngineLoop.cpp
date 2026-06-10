#include "EngineLoop.h"

#include "../Platform/Window.h"
#include "../Render/RenderSystem.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

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
    }

    EngineLoop::EngineLoop() = default;

    EngineLoop::~EngineLoop()
    {
        Shutdown();
    }

    bool EngineLoop::Initialize(const FApplicationDesc& desc)
    {
        Shutdown();

        mWindow = std::make_unique<Platform::Window>();

        Platform::WindowCreateInfo windowInfo {};
        windowInfo.Title = WideToUtf8(desc.Title);
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
        renderDesc.EnableVSync = desc.EnableVSync;

        renderDesc.ClearColor.R = 0.015f;
        renderDesc.ClearColor.G = 0.016f;
        renderDesc.ClearColor.B = 0.020f;
        renderDesc.ClearColor.A = 1.0f;

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

        return true;
    }

    int EngineLoop::Run()
    {
        if (!mInitialized)
            return -1;

        while (mRunning)
        {
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

            const auto currentTime = std::chrono::steady_clock::now();

            const std::chrono::duration<double> delta =
                currentTime - mLastFrameTime;

            mLastFrameTime = currentTime;

            const double deltaSeconds = delta.count();

            HandleResize();
            Tick(deltaSeconds);
            RenderFrame(deltaSeconds);

            ++mFrameIndex;
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

        if (mWindow)
        {
            mWindow->Destroy();
            mWindow.reset();
        }

        mFrameIndex = 0;
        mInitialized = false;
    }

    void EngineLoop::Tick(double deltaSeconds)
    {
        (void)deltaSeconds;

        // Позже тут будут:
        // input update
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
    }
}