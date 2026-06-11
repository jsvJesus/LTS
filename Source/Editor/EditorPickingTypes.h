#pragma once

#include "Core/BaseTypes.h"
#include "Core/Math/Math.h"

namespace Editor
{
    struct FEditorPickRay final
    {
        Core::Vector3 Origin = Core::Vector3::Zero();
        Core::Vector3 Direction = Core::Vector3::Forward();

        [[nodiscard]] bool IsValid() const
        {
            return Direction.LengthSquared() > 0.00001f;
        }
    };

    struct FEditorPickRequest final
    {
        Core::u64 RequestId = 0;

        Core::i32 MouseX = 0;
        Core::i32 MouseY = 0;

        Core::u32 ViewportWidth = 0;
        Core::u32 ViewportHeight = 0;

        Core::f32 NormalizedX = 0.0f;
        Core::f32 NormalizedY = 0.0f;

        FEditorPickRay Ray {};

        [[nodiscard]] bool IsValid() const
        {
            return RequestId != 0 && Ray.IsValid();
        }
    };
}