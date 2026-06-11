#include "EditorGizmoController.h"

#include "Platform/Input.h"

#include "Render/RenderSystem.h"
#include "Render/RHI/RenderTypes.h"

#include "Core/Logger.h"

#include <cmath>
#include <limits>

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

        Core::f32 Clamp01(const Core::f32 value)
        {
            if (value < 0.0f)
                return 0.0f;

            if (value > 1.0f)
                return 1.0f;

            return value;
        }

        Core::f32 DistanceSquared(
            const Core::Vector3& a,
            const Core::Vector3& b
        )
        {
            return (a - b).LengthSquared();
        }

        bool ComputeRaySegmentClosestPoints(
            const Core::Vector3& rayOrigin,
            const Core::Vector3& rayDirection,
            const Core::Vector3& segmentStart,
            const Core::Vector3& segmentEnd,
            Core::f32& outRayDistance,
            Core::Vector3& outRayPoint,
            Core::Vector3& outSegmentPoint
        )
        {
            const Core::Vector3 direction = rayDirection.Normalized();

            if (direction.LengthSquared() <= 0.00001f)
                return false;

            const Core::Vector3 segment = segmentEnd - segmentStart;
            const Core::f32 segmentLengthSquared = segment.LengthSquared();

            if (segmentLengthSquared <= 0.00001f)
                return false;

            const Core::Vector3 w0 = rayOrigin - segmentStart;

            const Core::f32 a = Core::Vector3::Dot(direction, direction);
            const Core::f32 b = Core::Vector3::Dot(direction, segment);
            const Core::f32 c = Core::Vector3::Dot(segment, segment);
            const Core::f32 d = Core::Vector3::Dot(direction, w0);
            const Core::f32 e = Core::Vector3::Dot(segment, w0);

            const Core::f32 denominator = a * c - b * b;

            Core::f32 rayDistance = 0.0f;
            Core::f32 segmentFactor = 0.0f;

            if (std::fabs(denominator) > 0.00001f)
            {
                rayDistance = (b * e - c * d) / denominator;
                segmentFactor = (a * e - b * d) / denominator;
            }
            else
            {
                rayDistance = 0.0f;
                segmentFactor = e / c;
            }

            segmentFactor = Clamp01(segmentFactor);

            Core::Vector3 segmentPoint = segmentStart + segment * segmentFactor;

            rayDistance = Core::Vector3::Dot(segmentPoint - rayOrigin, direction);

            if (rayDistance < 0.0f)
            {
                rayDistance = 0.0f;
            }

            Core::Vector3 rayPoint = rayOrigin + direction * rayDistance;

            segmentFactor =
                Core::Vector3::Dot(rayPoint - segmentStart, segment) / segmentLengthSquared;

            segmentFactor = Clamp01(segmentFactor);

            segmentPoint = segmentStart + segment * segmentFactor;

            rayDistance = Core::Vector3::Dot(segmentPoint - rayOrigin, direction);

            if (rayDistance < 0.0f)
            {
                rayDistance = 0.0f;
            }

            rayPoint = rayOrigin + direction * rayDistance;

            outRayDistance = rayDistance;
            outRayPoint = rayPoint;
            outSegmentPoint = segmentPoint;

            return true;
        }

        bool IntersectRaySphere(
            const Core::Vector3& rayOrigin,
            const Core::Vector3& rayDirection,
            const Core::Vector3& sphereCenter,
            const Core::f32 sphereRadius,
            Core::f32& outDistance
        )
        {
            const Core::Vector3 direction = rayDirection.Normalized();

            if (direction.LengthSquared() <= 0.00001f)
                return false;

            const Core::Vector3 oc = rayOrigin - sphereCenter;

            const Core::f32 a = Core::Vector3::Dot(direction, direction);
            const Core::f32 b = 2.0f * Core::Vector3::Dot(oc, direction);
            const Core::f32 c =
                Core::Vector3::Dot(oc, oc) - sphereRadius * sphereRadius;

            const Core::f32 discriminant = b * b - 4.0f * a * c;

            if (discriminant < 0.0f)
                return false;

            const Core::f32 sqrtDiscriminant = Core::SafeSqrt(discriminant);
            const Core::f32 invDenominator = 1.0f / (2.0f * a);

            const Core::f32 t0 = (-b - sqrtDiscriminant) * invDenominator;
            const Core::f32 t1 = (-b + sqrtDiscriminant) * invDenominator;

            if (t0 >= 0.0f)
            {
                outDistance = t0;
                return true;
            }

            if (t1 >= 0.0f)
            {
                outDistance = t1;
                return true;
            }

            return false;
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
        mAxisHitRadius = SanitizePositiveValue(desc.AxisHitRadius, 0.18f);

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
        mAxisHitRadius = 0.18f;
    }

    void EditorGizmoController::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized)
            return;

        if (mState.IsDragging() && mContext.InputSystem)
        {
            if (mContext.InputSystem->IsMouseButtonReleased(Platform::MouseButton::Left) ||
                !mContext.InputSystem->IsMouseButtonDown(Platform::MouseButton::Left))
            {
                EndDrag();
            }
        }
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

        if (mState.IsDragging())
        {
            EndDrag();
        }

        mState.ToolMode = toolMode;

        if (toolMode == EEditorToolMode::Select)
        {
            mState.ActiveAxis = EEditorGizmoAxis::None;
            mState.DragState = EEditorGizmoDragState::Idle;
            mState.HasValidDragStart = false;
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

        if (mState.IsDragging() && mState.TargetEntityId != entityId)
        {
            EndDrag();
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
        mState.DragStartTransform = World::FTransform {};
        mState.DragStartAxisValue = 0.0f;
        mState.HasValidDragStart = false;
    }

    bool EditorGizmoController::TryHitAxis(
        const FEditorPickRay& ray,
        FEditorGizmoAxisHitResult& outResult
    ) const
    {
        outResult = FEditorGizmoAxisHitResult {};
        outResult.Distance = std::numeric_limits<Core::f32>::max();

        if (!mInitialized || !ray.IsValid() || !mState.CanDrawGizmo())
            return false;

        bool hit = false;

        switch (mState.ToolMode)
        {
        case EEditorToolMode::Move:
            hit = TryHitMoveGizmoAxis(ray, outResult);
            break;

        case EEditorToolMode::Rotate:
            hit = TryHitRotateGizmoAxis(ray, outResult);
            break;

        case EEditorToolMode::Scale:
            hit = TryHitScaleGizmoAxis(ray, outResult);
            break;

        case EEditorToolMode::Select:
        default:
            hit = false;
            break;
        }

        return hit && outResult.IsValid();
    }

    void EditorGizmoController::BeginDrag(
        const FEditorPickRay& ray,
        const EEditorGizmoAxis axis
    )
    {
        if (!mState.CanDrawGizmo() || !ray.IsValid() || axis == EEditorGizmoAxis::None)
            return;

        // Этот этап Roadmap делает только Move preview.
        // Rotate/Scale будут отдельными следующими этапами.
        if (mState.ToolMode != EEditorToolMode::Move)
            return;

        const Core::Vector3 axisDirection = GetAxisDirection(axis);

        if (axisDirection.LengthSquared() <= 0.00001f)
            return;

        Core::f32 axisValue = 0.0f;

        if (!TryGetRayAxisPlaneValue(
            ray,
            mState.TargetTransform.Position,
            axisDirection,
            ray.Direction,
            axisValue
        ))
        {
            return;
        }

        mState.ActiveAxis = axis;
        mState.DragState = EEditorGizmoDragState::Dragging;
        mState.DragStartRay = ray;
        mState.LastRay = ray;

        mState.DragStartTransform = mState.TargetTransform;
        mState.DragStartAxisValue = axisValue;
        mState.HasValidDragStart = true;
    }

    void EditorGizmoController::UpdateDrag(const FEditorPickRay& ray)
    {
        if (!mState.IsDragging() || !ray.IsValid())
            return;

        mState.LastRay = ray;

        if (mState.ToolMode != EEditorToolMode::Move)
            return;

        if (!mState.HasValidDragStart)
            return;

        const Core::Vector3 axisDirection = GetAxisDirection(mState.ActiveAxis);

        if (axisDirection.LengthSquared() <= 0.00001f)
            return;

        Core::f32 currentAxisValue = 0.0f;

        if (!TryGetRayAxisPlaneValue(
            ray,
            mState.DragStartTransform.Position,
            axisDirection,
            mState.DragStartRay.Direction,
            currentAxisValue
        ))
        {
            return;
        }

        const Core::f32 delta = currentAxisValue - mState.DragStartAxisValue;

        mState.TargetTransform = mState.DragStartTransform;
        mState.TargetTransform.Position =
            mState.DragStartTransform.Position + axisDirection * delta;
    }

    void EditorGizmoController::EndDrag()
    {
        if (!mState.IsDragging())
            return;

        mState.ActiveAxis = EEditorGizmoAxis::None;
        mState.DragState = EEditorGizmoDragState::Idle;
        mState.DragStartRay = FEditorPickRay {};
        mState.LastRay = FEditorPickRay {};
        mState.DragStartTransform = World::FTransform {};
        mState.DragStartAxisValue = 0.0f;
        mState.HasValidDragStart = false;
    }

    bool EditorGizmoController::TryHitMoveGizmoAxis(
        const FEditorPickRay& ray,
        FEditorGizmoAxisHitResult& outResult
    ) const
    {
        const Core::Vector3 position = mState.TargetTransform.Position;
        const Core::f32 length = GetSafeMoveAxisLength();

        bool hit = false;

        hit |= TryHitAxisSegment(
            ray,
            position,
            position + Core::Vector3::Right() * length,
            EEditorGizmoAxis::X,
            outResult
        );

        hit |= TryHitAxisSegment(
            ray,
            position,
            position + Core::Vector3::Up() * length,
            EEditorGizmoAxis::Y,
            outResult
        );

        hit |= TryHitAxisSegment(
            ray,
            position,
            position + Core::Vector3::Forward() * length,
            EEditorGizmoAxis::Z,
            outResult
        );

        return hit;
    }

    bool EditorGizmoController::TryHitRotateGizmoAxis(
        const FEditorPickRay& ray,
        FEditorGizmoAxisHitResult& outResult
    ) const
    {
        const Core::Vector3 position = mState.TargetTransform.Position;
        const Core::f32 radius = GetSafeRotateRadius();

        bool hit = false;

        hit |= TryHitCircleSegments(
            ray,
            position,
            Core::Vector3::Up(),
            Core::Vector3::Forward(),
            radius,
            EEditorGizmoAxis::X,
            outResult
        );

        hit |= TryHitCircleSegments(
            ray,
            position,
            Core::Vector3::Right(),
            Core::Vector3::Forward(),
            radius,
            EEditorGizmoAxis::Y,
            outResult
        );

        hit |= TryHitCircleSegments(
            ray,
            position,
            Core::Vector3::Right(),
            Core::Vector3::Up(),
            radius,
            EEditorGizmoAxis::Z,
            outResult
        );

        return hit;
    }

    bool EditorGizmoController::TryHitScaleGizmoAxis(
        const FEditorPickRay& ray,
        FEditorGizmoAxisHitResult& outResult
    ) const
    {
        const Core::Vector3 position = mState.TargetTransform.Position;

        bool hit = false;

        Core::f32 uniformDistance = 0.0f;

        if (IntersectRaySphere(
            ray.Origin,
            ray.Direction,
            position,
            GetSafeScaleBoxHalfExtent() * 1.35f,
            uniformDistance
        ))
        {
            const Core::Vector3 hitPosition =
                ray.Origin + ray.Direction.Normalized() * uniformDistance;

            hit |= TryAcceptAxisHit(
                EEditorGizmoAxis::Uniform,
                uniformDistance,
                hitPosition,
                outResult
            );
        }

        const Core::f32 length = GetSafeMoveAxisLength() * 0.85f;

        hit |= TryHitAxisSegment(
            ray,
            position,
            position + Core::Vector3::Right() * length,
            EEditorGizmoAxis::X,
            outResult
        );

        hit |= TryHitAxisSegment(
            ray,
            position,
            position + Core::Vector3::Up() * length,
            EEditorGizmoAxis::Y,
            outResult
        );

        hit |= TryHitAxisSegment(
            ray,
            position,
            position + Core::Vector3::Forward() * length,
            EEditorGizmoAxis::Z,
            outResult
        );

        return hit;
    }

    bool EditorGizmoController::TryHitAxisSegment(
        const FEditorPickRay& ray,
        const Core::Vector3& start,
        const Core::Vector3& end,
        const EEditorGizmoAxis axis,
        FEditorGizmoAxisHitResult& outResult
    ) const
    {
        Core::f32 rayDistance = 0.0f;
        Core::Vector3 rayPoint = Core::Vector3::Zero();
        Core::Vector3 segmentPoint = Core::Vector3::Zero();

        if (!ComputeRaySegmentClosestPoints(
            ray.Origin,
            ray.Direction,
            start,
            end,
            rayDistance,
            rayPoint,
            segmentPoint
        ))
        {
            return false;
        }

        const Core::f32 hitRadius = GetSafeAxisHitRadius();
        const Core::f32 distanceSquared = DistanceSquared(rayPoint, segmentPoint);

        if (distanceSquared > hitRadius * hitRadius)
            return false;

        return TryAcceptAxisHit(axis, rayDistance, segmentPoint, outResult);
    }

    bool EditorGizmoController::TryHitCircleSegments(
        const FEditorPickRay& ray,
        const Core::Vector3& center,
        const Core::Vector3& axisA,
        const Core::Vector3& axisB,
        const Core::f32 radius,
        const EEditorGizmoAxis axis,
        FEditorGizmoAxisHitResult& outResult
    ) const
    {
        const Core::Vector3 normalizedAxisA = axisA.Normalized();
        const Core::Vector3 normalizedAxisB = axisB.Normalized();

        if (normalizedAxisA.LengthSquared() <= 0.00001f ||
            normalizedAxisB.LengthSquared() <= 0.00001f)
        {
            return false;
        }

        constexpr Core::i32 SegmentCount = 48;

        bool hit = false;

        Core::Vector3 previousPoint =
            center + normalizedAxisA * radius;

        for (Core::i32 segmentIndex = 1; segmentIndex <= SegmentCount; ++segmentIndex)
        {
            const Core::f32 t =
                static_cast<Core::f32>(segmentIndex) / static_cast<Core::f32>(SegmentCount);

            const Core::f32 angle = Core::TwoPi * t;

            const Core::f32 cosine = static_cast<Core::f32>(std::cos(angle));
            const Core::f32 sine = static_cast<Core::f32>(std::sin(angle));

            const Core::Vector3 currentPoint =
                center +
                normalizedAxisA * (cosine * radius) +
                normalizedAxisB * (sine * radius);

            hit |= TryHitAxisSegment(
                ray,
                previousPoint,
                currentPoint,
                axis,
                outResult
            );

            previousPoint = currentPoint;
        }

        return hit;
    }

    bool EditorGizmoController::TryAcceptAxisHit(
        const EEditorGizmoAxis axis,
        const Core::f32 distance,
        const Core::Vector3& hitPosition,
        FEditorGizmoAxisHitResult& outResult
    ) const
    {
        if (axis == EEditorGizmoAxis::None)
            return false;

        if (distance < 0.0f)
            return false;

        if (outResult.IsValid() && distance >= outResult.Distance)
            return false;

        outResult.Hit = true;
        outResult.Axis = axis;
        outResult.Distance = distance;
        outResult.HitPosition = hitPosition;

        return true;
    }

    Core::Vector3 EditorGizmoController::GetAxisDirection(const EEditorGizmoAxis axis) const
    {
        switch (axis)
        {
        case EEditorGizmoAxis::X:
            return Core::Vector3::Right();

        case EEditorGizmoAxis::Y:
            return Core::Vector3::Up();

        case EEditorGizmoAxis::Z:
            return Core::Vector3::Forward();

        case EEditorGizmoAxis::Uniform:
        case EEditorGizmoAxis::None:
        default:
            return Core::Vector3::Zero();
        }
    }

    bool EditorGizmoController::TryGetRayAxisPlaneValue(
        const FEditorPickRay& ray,
        const Core::Vector3& axisOrigin,
        const Core::Vector3& axisDirection,
        const Core::Vector3& referenceViewDirection,
        Core::f32& outValue
    ) const
    {
        outValue = 0.0f;

        if (!ray.IsValid())
            return false;

        const Core::Vector3 rayDirection = ray.Direction.Normalized();
        const Core::Vector3 axis = axisDirection.Normalized();
        const Core::Vector3 viewDirection = referenceViewDirection.Normalized();

        if (rayDirection.LengthSquared() <= 0.00001f ||
            axis.LengthSquared() <= 0.00001f ||
            viewDirection.LengthSquared() <= 0.00001f)
        {
            return false;
        }

        Core::Vector3 side = Core::Vector3::Cross(viewDirection, axis);

        if (side.LengthSquared() <= 0.00001f)
        {
            side = Core::Vector3::Cross(Core::Vector3::Up(), axis);
        }

        if (side.LengthSquared() <= 0.00001f)
        {
            side = Core::Vector3::Cross(Core::Vector3::Right(), axis);
        }

        if (side.LengthSquared() <= 0.00001f)
            return false;

        side.Normalize();

        Core::Vector3 planeNormal = Core::Vector3::Cross(axis, side);

        if (planeNormal.LengthSquared() <= 0.00001f)
            return false;

        planeNormal.Normalize();

        const Core::f32 denominator =
            Core::Vector3::Dot(rayDirection, planeNormal);

        if (std::fabs(denominator) <= 0.00001f)
            return false;

        const Core::f32 distance =
            Core::Vector3::Dot(axisOrigin - ray.Origin, planeNormal) / denominator;

        if (distance < 0.0f)
            return false;

        const Core::Vector3 hitPoint = ray.Origin + rayDirection * distance;

        outValue = Core::Vector3::Dot(hitPoint - axisOrigin, axis);
        return true;
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

    Core::f32 EditorGizmoController::GetSafeAxisHitRadius() const
    {
        return SanitizePositiveValue(mAxisHitRadius, 0.18f);
    }
}