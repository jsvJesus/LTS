#pragma once

#include "Core/BaseTypes.h"
#include "Core/Math/Math.h"

namespace Platform
{
    class InputSystem;
}

namespace Engine
{
    class Camera;

    struct FCameraControllerDesc final
    {
        bool EnableMovement = true;
        bool EnableRotation = true;

        bool RequireCursorLockForRotation = true;

        bool EnableSpeedBoost = true;
        Core::f32 MoveSpeed = 7.5f;
        Core::f32 SpeedBoostMultiplier = 4.0f;

        Core::f32 MouseSensitivity = 0.08f;
    };

    class CameraController final
    {
    public:
        CameraController() = default;
        ~CameraController() = default;

        CameraController(const CameraController&) = delete;
        CameraController& operator=(const CameraController&) = delete;

        bool Initialize(const FCameraControllerDesc& desc);
        void Reset();

        void Update(Camera& camera, const Platform::InputSystem& inputSystem, double deltaSeconds);

        void SetMovementEnabled(bool enabled);
        void SetRotationEnabled(bool enabled);
        void SetRequireCursorLockForRotation(bool required);
        void SetSpeedBoostEnabled(bool enabled);

        void SetMoveSpeed(Core::f32 moveSpeed);
        void SetSpeedBoostMultiplier(Core::f32 multiplier);
        void SetMouseSensitivity(Core::f32 sensitivity);

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }

        [[nodiscard]] bool IsMovementEnabled() const { return mEnableMovement; }
        [[nodiscard]] bool IsRotationEnabled() const { return mEnableRotation; }
        [[nodiscard]] bool IsCursorLockRequiredForRotation() const { return mRequireCursorLockForRotation; }
        [[nodiscard]] bool IsSpeedBoostEnabled() const { return mEnableSpeedBoost; }

        [[nodiscard]] Core::f32 GetMoveSpeed() const { return mMoveSpeed; }
        [[nodiscard]] Core::f32 GetSpeedBoostMultiplier() const { return mSpeedBoostMultiplier; }
        [[nodiscard]] Core::f32 GetMouseSensitivity() const { return mMouseSensitivity; }

    private:
        static Core::f32 SanitizeMoveSpeed(Core::f32 moveSpeed);
        static Core::f32 SanitizeSpeedBoostMultiplier(Core::f32 multiplier);
        static Core::f32 SanitizeMouseSensitivity(Core::f32 sensitivity);

    private:
        bool mInitialized = false;

        bool mEnableMovement = true;
        bool mEnableRotation = true;

        bool mRequireCursorLockForRotation = true;

        bool mEnableSpeedBoost = true;
        Core::f32 mMoveSpeed = 7.5f;
        Core::f32 mSpeedBoostMultiplier = 4.0f;

        Core::f32 mMouseSensitivity = 0.08f;
    };
}