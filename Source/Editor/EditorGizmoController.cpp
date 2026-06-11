#include "EditorGizmoController.h"

#include "Render/RenderSystem.h"
#include "Render/RHI/RenderTypes.h"

#include "Core/Logger.h"

#include <cmath>

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

    bool EditorGizmoController::Initialize(
        const Engine::FApplicationRuntimeContext& context,
        const FEditorGizmoControllerDesc& desc
    )
    {
        mContext = context;

        if (!mContext.RenderSystem)
        {
            mInitialized = false;
            return false;
        }

        mState = FEditorGizmoState {};

        mDebugDrawEnabled = desc.EnableDebugDraw;

        mMoveAxisLength = SanitizePositiveValue(desc.MoveAxisLength, 1.35f);
        mRotateRadius = SanitizePositiveValue(desc.RotateRadius, 0.85f);
        mScaleBoxHalfExtent = SanitizePositiveValue(desc.ScaleBoxHalfExtent, 0.42f);

        mInitialized = true;

        Core::Logger::Info("Editor", "Editor gizmo controller initialized.");

        return true;
    }

    void EditorGizmoController::Shutdown()
    {
        if (mInitialized)
        {
            Core::Logger::Info("Editor", "Editor gizmo controller shutdown.");
        }

        mContext = Engine::FApplicationRuntimeContext {};
        mState = FEditorGizmoState {};

        mInitialized = false;
        mDebugDrawEnabled = true;

        mMoveAxisLength = 1.35f;
        mRotateRadius = 0.85f;
        mScaleBoxHalfExtent = 0.42f;
    }

    void EditorGizmoController::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized)
            return;
    }

    void EditorGizmoController::RenderDebug()
    {
        if (!mInitialized || !mDebugDrawEnabled || !mContext.RenderSystem)
            return;

        DrawGizmo();
    }

    void EditorGizmoController::SetToolMode(const EEditorToolMode toolMode)
    {
        if (mState.ToolMode == toolMode)
            return;

        mState.ToolMode = toolMode;

        if (toolMode == EEditorToolMode::Select)
        {
            mState.ActiveAxis = EEditorGizmoAxis::None;
            mState.DragState = EEditorGizmoDragState::Idle;
        }
    }

    void EditorGizmoController::SetTarget(
        const World::EntityId entityId,
        const World::FTransform& transform
    )
    {
        if (!World::IsValidEntityId(entityId))
        {
            ClearTarget();
            return;
        }

        mState.TargetEntityId = entityId;
        mState.TargetTransform = transform;
        mState.HasTarget = true;
    }

    void EditorGizmoController::ClearTarget()
    {
        mState.TargetEntityId = World::InvalidEntityId;
        mState.TargetTransform = World::FTransform {};
        mState.HasTarget = false;
        mState.ActiveAxis = EEditorGizmoAxis::None;
        mState.DragState = EEditorGizmoDragState::Idle;
        mState.DragStartRay = FEditorPickRay {};
        mState.LastRay = FEditorPickRay {};
    }

    void EditorGizmoController::BeginDrag(
        const FEditorPickRay& ray,
        const EEditorGizmoAxis axis
    )
    {
        if (!mState.CanDrawGizmo() || !ray.IsValid())
            return;

        mState.ActiveAxis = axis;
        mState.DragState = EEditorGizmoDragState::Dragging;
        mState.DragStartRay = ray;
        mState.LastRay = ray;
    }

    void EditorGizmoController::UpdateDrag(const FEditorPickRay& ray)
    {
        if (!mState.IsDragging() || !ray.IsValid())
            return;

        mState.LastRay = ray;
    }

    void EditorGizmoController::EndDrag()
    {
        if (!mState.IsDragging())
            return;

        mState.ActiveAxis = EEditorGizmoAxis::None;
        mState.DragState = EEditorGizmoDragState::Idle;
        mState.DragStartRay = FEditorPickRay {};
        mState.LastRay = FEditorPickRay {};
    }

    void EditorGizmoController::DrawGizmo()
    {
        if (!mState.CanDrawGizmo())
            return;

        const Core::Vector3 position = mState.TargetTransform.Position;

        switch (mState.ToolMode)
        {
        case EEditorToolMode::Move:
            DrawMoveGizmo(position);
            break;

        case EEditorToolMode::Rotate:
            DrawRotateGizmo(position);
            break;

        case EEditorToolMode::Scale:
            DrawScaleGizmo(position);
            break;

        case EEditorToolMode::Select:
        default:
            break;
        }
    }

    void EditorGizmoController::DrawMoveGizmo(const Core::Vector3& position)
    {
        const Core::f32 length = GetSafeMoveAxisLength();
        const Core::f32 headSize = length * 0.18f;

        DrawDebugArrow(
            position,
            position + Core::Vector3::Right() * length,
            GetAxisColor(EEditorGizmoAxis::X),
            headSize
        );

        DrawDebugArrow(
            position,
            position + Core::Vector3::Up() * length,
            GetAxisColor(EEditorGizmoAxis::Y),
            headSize
        );

        DrawDebugArrow(
            position,
            position + Core::Vector3::Forward() * length,
            GetAxisColor(EEditorGizmoAxis::Z),
            headSize
        );
    }

    void EditorGizmoController::DrawRotateGizmo(const Core::Vector3& position)
    {
        const Core::f32 radius = GetSafeRotateRadius();

        DrawDebugCircle(
            position,
            Core::Vector3::Up(),
            Core::Vector3::Forward(),
            radius,
            GetAxisColor(EEditorGizmoAxis::X),
            48
        );

        DrawDebugCircle(
            position,
            Core::Vector3::Right(),
            Core::Vector3::Forward(),
            radius,
            GetAxisColor(EEditorGizmoAxis::Y),
            48
        );

        DrawDebugCircle(
            position,
            Core::Vector3::Right(),
            Core::Vector3::Up(),
            radius,
            GetAxisColor(EEditorGizmoAxis::Z),
            48
        );
    }

    void EditorGizmoController::DrawScaleGizmo(const Core::Vector3& position)
    {
        const Render::FRenderColor uniformColor =
            GetAxisColor(EEditorGizmoAxis::Uniform);

        DrawDebugBox(position, GetSafeScaleBoxHalfExtent(), uniformColor);

        const Core::f32 length = GetSafeMoveAxisLength() * 0.85f;

        if (!mContext.RenderSystem)
            return;

        mContext.RenderSystem->DrawDebugLine(
            position,
            position + Core::Vector3::Right() * length,
            GetAxisColor(EEditorGizmoAxis::X)
        );

        mContext.RenderSystem->DrawDebugLine(
            position,
            position + Core::Vector3::Up() * length,
            GetAxisColor(EEditorGizmoAxis::Y)
        );

        mContext.RenderSystem->DrawDebugLine(
            position,
            position + Core::Vector3::Forward() * length,
            GetAxisColor(EEditorGizmoAxis::Z)
        );
    }

    void EditorGizmoController::DrawDebugArrow(
        const Core::Vector3& start,
        const Core::Vector3& end,
        const Render::FRenderColor& color,
        const Core::f32 headSize
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 direction = (end - start).Normalized();

        if (direction.LengthSquared() <= 0.00001f)
            return;

        Core::Vector3 side = Core::Vector3::Cross(direction, Core::Vector3::Up());

        if (side.LengthSquared() <= 0.00001f)
        {
            side = Core::Vector3::Cross(direction, Core::Vector3::Right());
        }

        side.Normalize();

        const Core::Vector3 back = end - direction * headSize;

        mContext.RenderSystem->DrawDebugLine(start, end, color);
        mContext.RenderSystem->DrawDebugLine(end, back + side * (headSize * 0.55f), color);
        mContext.RenderSystem->DrawDebugLine(end, back - side * (headSize * 0.55f), color);
    }

    void EditorGizmoController::DrawDebugCircle(
        const Core::Vector3& center,
        const Core::Vector3& axisA,
        const Core::Vector3& axisB,
        const Core::f32 radius,
        const Render::FRenderColor& color,
        const Core::i32 segmentCount
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 normalizedAxisA = axisA.Normalized();
        const Core::Vector3 normalizedAxisB = axisB.Normalized();

        if (normalizedAxisA.LengthSquared() <= 0.00001f ||
            normalizedAxisB.LengthSquared() <= 0.00001f)
        {
            return;
        }

        const Core::i32 safeSegmentCount = segmentCount >= 3 ? segmentCount : 3;

        Core::Vector3 previousPoint =
            center + normalizedAxisA * radius;

        for (Core::i32 segmentIndex = 1; segmentIndex <= safeSegmentCount; ++segmentIndex)
        {
            const Core::f32 t =
                static_cast<Core::f32>(segmentIndex) / static_cast<Core::f32>(safeSegmentCount);

            const Core::f32 angle = Core::TwoPi * t;

            const Core::f32 cosine = static_cast<Core::f32>(std::cos(angle));
            const Core::f32 sine = static_cast<Core::f32>(std::sin(angle));

            const Core::Vector3 currentPoint =
                center +
                normalizedAxisA * (cosine * radius) +
                normalizedAxisB * (sine * radius);

            mContext.RenderSystem->DrawDebugLine(previousPoint, currentPoint, color);

            previousPoint = currentPoint;
        }
    }

    void EditorGizmoController::DrawDebugBox(
        const Core::Vector3& center,
        const Core::f32 halfExtent,
        const Render::FRenderColor& color
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 x = Core::Vector3::Right() * halfExtent;
        const Core::Vector3 y = Core::Vector3::Up() * halfExtent;
        const Core::Vector3 z = Core::Vector3::Forward() * halfExtent;

        const Core::Vector3 p000 = center - x - y - z;
        const Core::Vector3 p001 = center - x - y + z;
        const Core::Vector3 p010 = center - x + y - z;
        const Core::Vector3 p011 = center - x + y + z;

        const Core::Vector3 p100 = center + x - y - z;
        const Core::Vector3 p101 = center + x - y + z;
        const Core::Vector3 p110 = center + x + y - z;
        const Core::Vector3 p111 = center + x + y + z;

        mContext.RenderSystem->DrawDebugLine(p000, p001, color);
        mContext.RenderSystem->DrawDebugLine(p001, p011, color);
        mContext.RenderSystem->DrawDebugLine(p011, p010, color);
        mContext.RenderSystem->DrawDebugLine(p010, p000, color);

        mContext.RenderSystem->DrawDebugLine(p100, p101, color);
        mContext.RenderSystem->DrawDebugLine(p101, p111, color);
        mContext.RenderSystem->DrawDebugLine(p111, p110, color);
        mContext.RenderSystem->DrawDebugLine(p110, p100, color);

        mContext.RenderSystem->DrawDebugLine(p000, p100, color);
        mContext.RenderSystem->DrawDebugLine(p001, p101, color);
        mContext.RenderSystem->DrawDebugLine(p010, p110, color);
        mContext.RenderSystem->DrawDebugLine(p011, p111, color);
    }

    Render::FRenderColor EditorGizmoController::GetAxisColor(const EEditorGizmoAxis axis) const
    {
        const bool highlight = mState.ActiveAxis == axis;

        switch (axis)
        {
        case EEditorGizmoAxis::X:
            return highlight
                ? MakeColor(1.00f, 0.65f, 0.35f, 1.0f)
                : MakeColor(1.00f, 0.20f, 0.20f, 1.0f);

        case EEditorGizmoAxis::Y:
            return highlight
                ? MakeColor(0.65f, 1.00f, 0.65f, 1.0f)
                : MakeColor(0.20f, 1.00f, 0.30f, 1.0f);

        case EEditorGizmoAxis::Z:
            return highlight
                ? MakeColor(0.55f, 0.75f, 1.00f, 1.0f)
                : MakeColor(0.20f, 0.45f, 1.00f, 1.0f);

        case EEditorGizmoAxis::Uniform:
            return highlight
                ? MakeColor(1.00f, 0.85f, 1.00f, 1.0f)
                : MakeColor(0.85f, 0.35f, 1.00f, 1.0f);

        case EEditorGizmoAxis::None:
        default:
            return MakeColor(0.90f, 0.90f, 0.90f, 1.0f);
        }
    }

    Core::f32 EditorGizmoController::GetSafeMoveAxisLength() const
    {
        return SanitizePositiveValue(mMoveAxisLength, 1.35f);
    }

    Core::f32 EditorGizmoController::GetSafeRotateRadius() const
    {
        return SanitizePositiveValue(mRotateRadius, 0.85f);
    }

    Core::f32 EditorGizmoController::GetSafeScaleBoxHalfExtent() const
    {
        return SanitizePositiveValue(mScaleBoxHalfExtent, 0.42f);
    }
}