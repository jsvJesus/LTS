#pragma once

#include "EditorGizmoTypes.h"

#include "Engine/ApplicationRuntime.h"

namespace Render
{
    struct FRenderColor;
}

namespace Editor
{
    struct FEditorGizmoControllerDesc final
    {
        bool EnableDebugDraw = true;

        Core::f32 MoveAxisLength = 1.35f;
        Core::f32 RotateRadius = 0.85f;
        Core::f32 ScaleBoxHalfExtent = 0.42f;
    };

    class EditorGizmoController final
    {
    public:
        EditorGizmoController() = default;
        ~EditorGizmoController() = default;

        EditorGizmoController(const EditorGizmoController&) = delete;
        EditorGizmoController& operator=(const EditorGizmoController&) = delete;

        bool Initialize(
            const Engine::FApplicationRuntimeContext& context,
            const FEditorGizmoControllerDesc& desc
        );

        void Shutdown();

        void Tick(double deltaSeconds);
        void RenderDebug();

        void SetToolMode(EEditorToolMode toolMode);

        void SetTarget(
            World::EntityId entityId,
            const World::FTransform& transform
        );

        void ClearTarget();

        void BeginDrag(const FEditorPickRay& ray, EEditorGizmoAxis axis);
        void UpdateDrag(const FEditorPickRay& ray);
        void EndDrag();

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }
        [[nodiscard]] const FEditorGizmoState& GetState() const { return mState; }

    private:
        void DrawGizmo();

        void DrawMoveGizmo(const Core::Vector3& position);
        void DrawRotateGizmo(const Core::Vector3& position);
        void DrawScaleGizmo(const Core::Vector3& position);

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

        [[nodiscard]] Render::FRenderColor GetAxisColor(EEditorGizmoAxis axis) const;
        [[nodiscard]] Core::f32 GetSafeMoveAxisLength() const;
        [[nodiscard]] Core::f32 GetSafeRotateRadius() const;
        [[nodiscard]] Core::f32 GetSafeScaleBoxHalfExtent() const;

    private:
        Engine::FApplicationRuntimeContext mContext {};

        FEditorGizmoState mState {};

        bool mInitialized = false;
        bool mDebugDrawEnabled = true;

        Core::f32 mMoveAxisLength = 1.35f;
        Core::f32 mRotateRadius = 0.85f;
        Core::f32 mScaleBoxHalfExtent = 0.42f;
    };
}