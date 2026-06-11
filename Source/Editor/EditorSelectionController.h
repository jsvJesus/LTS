#pragma once

#include "EditorSelectionTypes.h"

#include "Engine/ApplicationRuntime.h"

#include "Core/Math/Math.h"

namespace Render
{
    struct FRenderColor;
}

namespace Editor
{
    struct FEditorSelectionControllerDesc final
    {
        bool EnableDebugMarker = true;

        Core::f32 MarkerDistance = 4.25f;
        Core::f32 MarkerSize = 0.45f;
    };

    class EditorSelectionController final
    {
    public:
        EditorSelectionController() = default;
        ~EditorSelectionController() = default;

        EditorSelectionController(const EditorSelectionController&) = delete;
        EditorSelectionController& operator=(const EditorSelectionController&) = delete;

        bool Initialize(
            const Engine::FApplicationRuntimeContext& context,
            const FEditorSelectionControllerDesc& desc
        );

        void Shutdown();

        void Tick(double deltaSeconds);
        void RenderDebug();

        bool SetSelectedId(EditorSelectionId selectedId);
        bool ClearSelection();

        void SetDebugMarkerEnabled(bool enabled);

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }
        [[nodiscard]] bool IsDebugMarkerEnabled() const { return mDebugMarkerEnabled; }

        [[nodiscard]] bool HasSelection() const { return mSelectionState.HasSelection(); }
        [[nodiscard]] EditorSelectionId GetSelectedId() const { return mSelectionState.SelectedId; }
        [[nodiscard]] const FEditorSelectionState& GetSelectionState() const { return mSelectionState; }

    private:
        void HandleHotkeys();

        void LogSelectionChanged() const;
        void LogSelectionCleared() const;

        void DrawSelectionStateMarker();

        void DrawNoSelectionMarker(
            const Core::Vector3& center,
            Core::f32 size
        );

        void DrawSelectedMarker(
            const Core::Vector3& center,
            Core::f32 size
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

        FEditorSelectionState mSelectionState {};

        bool mInitialized = false;
        bool mDebugMarkerEnabled = true;

        Core::f32 mMarkerDistance = 4.25f;
        Core::f32 mMarkerSize = 0.45f;
    };
}