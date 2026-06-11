#include "EditorViewportController.h"

#include "Engine/Camera/Camera.h"
#include "Engine/Camera/CameraController.h"

#include "Platform/Input.h"

#include "Render/RenderSystem.h"

#include "Core/Logger.h"

namespace Editor
{
    namespace
    {
        Render::FRenderColor MakeColor(
            const float r,
            const float g,
            const float b,
            const float a
        )
        {
            Render::FRenderColor color {};
            color.R = r;
            color.G = g;
            color.B = b;
            color.A = a;
            return color;
        }

        Core::f32 SanitizePositiveValue(const Core::f32 value, const Core::f32 fallback)
        {
            return value > 0.001f ? value : fallback;
        }
    }

    bool EditorViewportController::Initialize(
        const Engine::FApplicationRuntimeContext& context,
        const FEditorViewportControllerDesc& desc
    )
    {
        mContext = context;

        if (!mContext.RenderSystem)
        {
            mInitialized = false;
            return false;
        }

        mViewportCameraEnabled = desc.EnableViewportCamera;
        mDebugOverlayEnabled = desc.EnableDebugOverlay;

        mDrawFocusMarker = desc.DrawFocusMarker;
        mDrawCameraForwardLine = desc.DrawCameraForwardLine;

        mFocusDistance = SanitizePositiveValue(desc.FocusDistance, 5.0f);
        mFocusMarkerSize = SanitizePositiveValue(desc.FocusMarkerSize, 1.0f);

        mInitialized = true;

        ApplyCameraControllerState();

        Core::Logger::Info("Editor", "Editor viewport controller initialized.");

        return true;
    }

    void EditorViewportController::Shutdown()
    {
        if (mInitialized)
        {
            Core::Logger::Info("Editor", "Editor viewport controller shutdown.");
        }

        mContext = Engine::FApplicationRuntimeContext {};

        mInitialized = false;

        mViewportCameraEnabled = true;
        mDebugOverlayEnabled = true;

        mDrawFocusMarker = true;
        mDrawCameraForwardLine = true;

        mFocusDistance = 5.0f;
        mFocusMarkerSize = 1.0f;
    }

    void EditorViewportController::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized || !mContext.InputSystem)
            return;

        if (mContext.InputSystem->IsKeyPressed(Platform::KeyCode::F7))
        {
            ToggleDebugOverlay();

            Core::Logger::Info(
                "Editor",
                mDebugOverlayEnabled
                    ? "Editor viewport debug overlay enabled."
                    : "Editor viewport debug overlay disabled."
            );
        }

        if (mContext.InputSystem->IsKeyPressed(Platform::KeyCode::F8))
        {
            ToggleViewportCamera();

            Core::Logger::Info(
                "Editor",
                mViewportCameraEnabled
                    ? "Editor viewport camera enabled."
                    : "Editor viewport camera disabled."
            );
        }
    }

    void EditorViewportController::RenderDebug()
    {
        if (!mInitialized || !mDebugOverlayEnabled || !mContext.RenderSystem)
            return;

        if (mDrawFocusMarker)
        {
            DrawFocusMarker();
        }

        if (mDrawCameraForwardLine)
        {
            DrawCameraForwardLine();
        }
    }

    void EditorViewportController::SetViewportCameraEnabled(const bool enabled)
    {
        mViewportCameraEnabled = enabled;
        ApplyCameraControllerState();
    }

    void EditorViewportController::SetDebugOverlayEnabled(const bool enabled)
    {
        mDebugOverlayEnabled = enabled;
    }

    bool EditorViewportController::ToggleViewportCamera()
    {
        SetViewportCameraEnabled(!mViewportCameraEnabled);
        return mViewportCameraEnabled;
    }

    bool EditorViewportController::ToggleDebugOverlay()
    {
        SetDebugOverlayEnabled(!mDebugOverlayEnabled);
        return mDebugOverlayEnabled;
    }

    void EditorViewportController::ApplyCameraControllerState()
    {
        if (!mContext.MainCameraController)
            return;

        mContext.MainCameraController->SetMovementEnabled(mViewportCameraEnabled);
        mContext.MainCameraController->SetRotationEnabled(mViewportCameraEnabled);
    }

    void EditorViewportController::DrawFocusMarker()
    {
        if (!mContext.RenderSystem || !mContext.MainCamera)
            return;

        const Core::Vector3 focusPoint = GetFocusPoint();

        const Core::Vector3 right =
            mContext.MainCamera->GetRightVector() * mFocusMarkerSize;

        const Core::Vector3 up =
            Core::Vector3::Up() * mFocusMarkerSize;

        const Render::FRenderColor accentColor =
            MakeColor(0.10f, 0.85f, 1.00f, 1.0f);

        const Render::FRenderColor softColor =
            MakeColor(0.08f, 0.45f, 0.55f, 1.0f);

        mContext.RenderSystem->DrawDebugLine(
            focusPoint - right,
            focusPoint + right,
            accentColor
        );

        mContext.RenderSystem->DrawDebugLine(
            focusPoint - up,
            focusPoint + up,
            accentColor
        );

        const Core::Vector3 markerA = focusPoint - right - up;
        const Core::Vector3 markerB = focusPoint + right - up;
        const Core::Vector3 markerC = focusPoint + right + up;
        const Core::Vector3 markerD = focusPoint - right + up;

        mContext.RenderSystem->DrawDebugLine(markerA, markerB, softColor);
        mContext.RenderSystem->DrawDebugLine(markerB, markerC, softColor);
        mContext.RenderSystem->DrawDebugLine(markerC, markerD, softColor);
        mContext.RenderSystem->DrawDebugLine(markerD, markerA, softColor);
    }

    void EditorViewportController::DrawCameraForwardLine()
    {
        if (!mContext.RenderSystem || !mContext.MainCamera)
            return;

        const Core::Vector3 cameraPosition = mContext.MainCamera->GetPosition();
        const Core::Vector3 cameraForward = mContext.MainCamera->GetForwardVector();

        const Render::FRenderColor forwardColor =
            MakeColor(0.10f, 0.85f, 1.00f, 1.0f);

        mContext.RenderSystem->DrawDebugLine(
            cameraPosition + cameraForward * 0.5f,
            cameraPosition + cameraForward * mFocusDistance,
            forwardColor
        );
    }

    Core::Vector3 EditorViewportController::GetFocusPoint() const
    {
        if (!mContext.MainCamera)
            return Core::Vector3::Zero();

        return mContext.MainCamera->GetPosition() +
            mContext.MainCamera->GetForwardVector() * mFocusDistance;
    }
}