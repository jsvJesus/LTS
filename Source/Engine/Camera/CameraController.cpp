#include "CameraController.h"

#include "Camera.h"

#include "Platform/Input.h"

namespace Engine
{
    namespace
    {
        constexpr Core::f32 MinMoveSpeed = 0.0f;
        constexpr Core::f32 MaxMoveSpeed = 10000.0f;

        constexpr Core::f32 MinSpeedBoostMultiplier = 1.0f;
        constexpr Core::f32 MaxSpeedBoostMultiplier = 32.0f;

        constexpr Core::f32 MinMouseSensitivity = 0.0f;
        constexpr Core::f32 MaxMouseSensitivity = 10.0f;
    }

    bool CameraController::Initialize(const FCameraControllerDesc& desc)
    {
        mEnableMovement = desc.EnableMovement;
        mEnableRotation = desc.EnableRotation;

        mRequireCursorLockForRotation = desc.RequireCursorLockForRotation;

        mEnableSpeedBoost = desc.EnableSpeedBoost;
        mMoveSpeed = SanitizeMoveSpeed(desc.MoveSpeed);
        mSpeedBoostMultiplier = SanitizeSpeedBoostMultiplier(desc.SpeedBoostMultiplier);

        mMouseSensitivity = SanitizeMouseSensitivity(desc.MouseSensitivity);

        mInitialized = true;
        return true;
    }

    void CameraController::Reset()
    {
        FCameraControllerDesc desc {};
        Initialize(desc);
    }

    void CameraController::Update(
        Camera& camera,
        const Platform::InputSystem& inputSystem,
        const double deltaSeconds
    )
    {
        if (!mInitialized || !camera.IsInitialized())
            return;

        const Core::f32 safeDeltaSeconds = Core::Clamp(
            static_cast<Core::f32>(deltaSeconds),
            0.0f,
            0.25f
        );

        if (mEnableRotation)
        {
            const bool canRotate =
                !mRequireCursorLockForRotation || inputSystem.IsCursorLocked();

            if (canRotate)
            {
                const Core::i32 rawMouseDeltaX = inputSystem.GetRawMouseDeltaX();
                const Core::i32 rawMouseDeltaY = inputSystem.GetRawMouseDeltaY();

                if (rawMouseDeltaX != 0 || rawMouseDeltaY != 0)
                {
                    Core::Rotator rotation = camera.GetRotation();

                    rotation.Yaw += static_cast<Core::f32>(rawMouseDeltaX) * mMouseSensitivity;
                    rotation.Pitch += static_cast<Core::f32>(rawMouseDeltaY) * mMouseSensitivity;

                    camera.SetRotation(rotation);
                }
            }
        }

        if (!mEnableMovement)
            return;

        Core::Vector3 movement = Core::Vector3::Zero();

        if (inputSystem.IsKeyDown(Platform::KeyCode::W))
            movement += camera.GetForwardVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::S))
            movement -= camera.GetForwardVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::D))
            movement += camera.GetRightVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::A))
            movement -= camera.GetRightVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::Space))
            movement += Core::Vector3::Up();

        if (inputSystem.IsKeyDown(Platform::KeyCode::LeftControl) ||
            inputSystem.IsKeyDown(Platform::KeyCode::RightControl))
        {
            movement -= Core::Vector3::Up();
        }

        if (movement.LengthSquared() <= 0.00001f)
            return;

        movement.Normalize();

        Core::f32 finalMoveSpeed = mMoveSpeed;

        if (mEnableSpeedBoost &&
            (inputSystem.IsKeyDown(Platform::KeyCode::LeftShift) ||
             inputSystem.IsKeyDown(Platform::KeyCode::RightShift)))
        {
            finalMoveSpeed *= mSpeedBoostMultiplier;
        }

        const Core::Vector3 newPosition =
            camera.GetPosition() + movement * (finalMoveSpeed * safeDeltaSeconds);

        camera.SetPosition(newPosition);
    }

    void CameraController::SetMovementEnabled(const bool enabled)
    {
        mEnableMovement = enabled;
    }

    void CameraController::SetRotationEnabled(const bool enabled)
    {
        mEnableRotation = enabled;
    }

    void CameraController::SetRequireCursorLockForRotation(const bool required)
    {
        mRequireCursorLockForRotation = required;
    }

    void CameraController::SetSpeedBoostEnabled(const bool enabled)
    {
        mEnableSpeedBoost = enabled;
    }

    void CameraController::SetMoveSpeed(const Core::f32 moveSpeed)
    {
        mMoveSpeed = SanitizeMoveSpeed(moveSpeed);
    }

    void CameraController::SetSpeedBoostMultiplier(const Core::f32 multiplier)
    {
        mSpeedBoostMultiplier = SanitizeSpeedBoostMultiplier(multiplier);
    }

    void CameraController::SetMouseSensitivity(const Core::f32 sensitivity)
    {
        mMouseSensitivity = SanitizeMouseSensitivity(sensitivity);
    }

    Core::f32 CameraController::SanitizeMoveSpeed(const Core::f32 moveSpeed)
    {
        return Core::Clamp(moveSpeed, MinMoveSpeed, MaxMoveSpeed);
    }

    Core::f32 CameraController::SanitizeSpeedBoostMultiplier(const Core::f32 multiplier)
    {
        return Core::Clamp(multiplier, MinSpeedBoostMultiplier, MaxSpeedBoostMultiplier);
    }

    Core::f32 CameraController::SanitizeMouseSensitivity(const Core::f32 sensitivity)
    {
        return Core::Clamp(sensitivity, MinMouseSensitivity, MaxMouseSensitivity);
    }
}