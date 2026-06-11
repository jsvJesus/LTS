#pragma once

#include "Engine/ApplicationRuntime.h"

#include "Core/BaseTypes.h"
#include "Core/Math/Math.h"

namespace Editor
{
    struct FEditorViewportControllerDesc final
    {
        bool EnableViewportCamera = true;
        bool EnableDebugOverlay = true;

        bool DrawFocusMarker = true;
        bool DrawCameraForwardLine = true;

        Core::f32 FocusDistance = 5.0f;
        Core::f32 FocusMarkerSize = 1.0f;
    };

    class EditorViewportController final
    {
    public:
        EditorViewportController() = default;
        ~EditorViewportController() = default;

        EditorViewportController(const EditorViewportController&) = delete;
        EditorViewportController& operator=(const EditorViewportController&) = delete;

        bool Initialize(
            const Engine::FApplicationRuntimeContext& context,
            const FEditorViewportControllerDesc& desc
        );

        void Shutdown();

        void Tick(double deltaSeconds);
        void RenderDebug();

        void SetViewportCameraEnabled(bool enabled);
        void SetDebugOverlayEnabled(bool enabled);

        bool ToggleViewportCamera();
        bool ToggleDebugOverlay();

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }
        [[nodiscard]] bool IsViewportCameraEnabled() const { return mViewportCameraEnabled; }
        [[nodiscard]] bool IsDebugOverlayEnabled() const { return mDebugOverlayEnabled; }

    private:
        void ApplyCameraControllerState();

        void DrawFocusMarker();
        void DrawCameraForwardLine();

        [[nodiscard]] Core::Vector3 GetFocusPoint() const;

    private:
        Engine::FApplicationRuntimeContext mContext {};

        bool mInitialized = false;

        bool mViewportCameraEnabled = true;
        bool mDebugOverlayEnabled = true;

        bool mDrawFocusMarker = true;
        bool mDrawCameraForwardLine = true;

        Core::f32 mFocusDistance = 5.0f;
        Core::f32 mFocusMarkerSize = 1.0f;
    };
}