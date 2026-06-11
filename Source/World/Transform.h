#pragma once

#include "Core/Math/Math.h"

namespace World
{
    struct FTransform final
    {
        Core::Vector3 Position = Core::Vector3::Zero();
        Core::Rotator Rotation = Core::Rotator::Zero();
        Core::Vector3 Scale = Core::Vector3::One();

        [[nodiscard]] Core::Matrix4 GetMatrix() const
        {
            const Core::Matrix4 scaleMatrix =
                Core::Matrix4::CreateScale(Scale);

            const Core::Matrix4 rotationMatrix =
                Rotation.ToMatrix();

            const Core::Matrix4 translationMatrix =
                Core::Matrix4::CreateTranslation(Position);

            // Row-major + translation in M[3][0..2].
            // Local row-vector transform: Local * Scale * Rotation * Translation.
            return scaleMatrix * rotationMatrix * translationMatrix;
        }

        [[nodiscard]] Core::Vector3 GetForwardVector() const
        {
            return Rotation.GetForwardVector();
        }

        [[nodiscard]] Core::Vector3 GetRightVector() const
        {
            return Rotation.GetRightVector();
        }

        [[nodiscard]] Core::Vector3 GetUpVector() const
        {
            return Rotation.GetUpVector();
        }
    };
}