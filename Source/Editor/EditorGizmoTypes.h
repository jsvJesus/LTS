#pragma once

#include "EditorToolMode.h"
#include "EditorPickingTypes.h"

#include "World/EntityId.h"
#include "World/Transform.h"

namespace Editor
{
    enum class EEditorGizmoAxis : Core::u8
    {
        None = 0,
        X,
        Y,
        Z,
        Uniform
    };

    enum class EEditorGizmoDragState : Core::u8
    {
        Idle = 0,
        Hover,
        Dragging
    };

    struct FEditorGizmoState final
    {
        EEditorToolMode ToolMode = EEditorToolMode::Select;

        EEditorGizmoAxis ActiveAxis = EEditorGizmoAxis::None;
        EEditorGizmoDragState DragState = EEditorGizmoDragState::Idle;

        World::EntityId TargetEntityId = World::InvalidEntityId;
        World::FTransform TargetTransform {};

        FEditorPickRay DragStartRay {};
        FEditorPickRay LastRay {};

        bool HasTarget = false;

        [[nodiscard]] bool CanDrawGizmo() const
        {
            return HasTarget &&
                World::IsValidEntityId(TargetEntityId) &&
                ToolMode != EEditorToolMode::Select;
        }

        [[nodiscard]] bool IsDragging() const
        {
            return DragState == EEditorGizmoDragState::Dragging;
        }
    };
}