#pragma once

#include "EditorToolMode.h"

#include "Engine/ApplicationRuntime.h"

#include "Core/Math/Math.h"

namespace Render
{
    struct FRenderColor;
}

namespace Editor
{
    struct FEditorToolModeControllerDesc final
    {
        EEditorToolMode InitialToolMode = EEditorToolMode::Select;

        bool EnableDebugMarker = true;

        Core::f32 MarkerDistance = 5.0f;
        Core::f32 MarkerSize = 0.75f;
    };

    class EditorToolModeController final
    {
    public:
        EditorToolModeController() = default;
        ~EditorToolModeController() = default;

        EditorToolModeController(const EditorToolModeController&) = delete;
        EditorToolModeController& operator=(const EditorToolModeController&) = delete;

        bool Initialize(
            const Engine::FApplicationRuntimeContext& context,
            const FEditorToolModeControllerDesc& desc
        );

        void Shutdown();

        void Tick(double deltaSeconds);
        void RenderDebug();

        bool SetToolMode(EEditorToolMode mode);

        void SetDebugMarkerEnabled(bool enabled);

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }
        [[nodiscard]] bool IsDebugMarkerEnabled() const { return mDebugMarkerEnabled; }

        [[nodiscard]] EEditorToolMode GetToolMode() const { return mCurrentToolMode; }
        [[nodiscard]] Core::StringView GetToolModeName() const { return GetEditorToolModeName(mCurrentToolMode); }

    private:
        void HandleHotkeys();

        void LogToolModeChanged() const;

        void DrawCurrentModeMarker();

        void DrawSelectMarker(const Core::Vector3& center, Core::f32 size);
        void DrawMoveMarker(const Core::Vector3& center, Core::f32 size);
        void DrawRotateMarker(const Core::Vector3& center, Core::f32 size);
        void DrawScaleMarker(const Core::Vector3& center, Core::f32 size);

        void DrawDebugArrow(
            const Core::Vector3& start,
            const Core::Vector3& end,
            const Render::FRenderColor& color,
            Core::f32 headSize
        );

        void DrawDebugCircle(
            const Core::Vector3& center,
            const Core::Vector3& axisA,
            const Core::Vector3& axisB,
            Core::f32 radius,
            const Render::FRenderColor& color,
            Core::i32 segmentCount
        );

        void DrawDebugBox(
            const Core::Vector3& center,
            Core::f32 halfExtent,
            const Render::FRenderColor& color
        );

        [[nodiscard]] Core::Vector3 GetMarkerCenter() const;
        [[nodiscard]] Core::Vector3 GetCameraRightVector() const;
        [[nodiscard]] Core::Vector3 GetCameraUpVector() const;

    private:
        Engine::FApplicationRuntimeContext mContext {};

        EEditorToolMode mCurrentToolMode = EEditorToolMode::Select;

        bool mInitialized = false;
        bool mDebugMarkerEnabled = true;

        Core::f32 mMarkerDistance = 5.0f;
        Core::f32 mMarkerSize = 0.75f;
    };
}