#pragma once

#include "Core/BaseTypes.h"
#include "Core/Math/Math.h"

namespace Platform
{
    class InputSystem;
}

namespace Engine
{
    struct FCameraDesc final
    {
        Core::Vector3 Position = Core::Vector3(0.0f, 1.6f, -5.0f);
        Core::Rotator Rotation = Core::Rotator::Zero();

        Core::f32 FieldOfViewYDegrees = 75.0f;
        Core::f32 NearPlane = 0.05f;
        Core::f32 FarPlane = 10000.0f;
        Core::f32 AspectRatio = 16.0f / 9.0f;

        Core::f32 MoveSpeed = 7.5f;
        Core::f32 MouseSensitivity = 0.08f;
    };

    class Camera final
    {
    public:
        Camera() = default;
        ~Camera() = default;

        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;

        bool Initialize(const FCameraDesc& desc);
        void Reset();

        void UpdateFromInput(const Platform::InputSystem& inputSystem, double deltaSeconds);

        void SetPosition(const Core::Vector3& position);
        void SetRotation(const Core::Rotator& rotation);

        void SetFieldOfViewYDegrees(Core::f32 fieldOfViewYDegrees);
        void SetClippingPlanes(Core::f32 nearPlane, Core::f32 farPlane);
        void SetAspectRatio(Core::f32 aspectRatio);

        void SetMoveSpeed(Core::f32 moveSpeed);
        void SetMouseSensitivity(Core::f32 mouseSensitivity);

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }

        [[nodiscard]] const Core::Vector3& GetPosition() const { return mPosition; }
        [[nodiscard]] const Core::Rotator& GetRotation() const { return mRotation; }

        [[nodiscard]] Core::f32 GetFieldOfViewYDegrees() const { return mFieldOfViewYDegrees; }
        [[nodiscard]] Core::f32 GetNearPlane() const { return mNearPlane; }
        [[nodiscard]] Core::f32 GetFarPlane() const { return mFarPlane; }
        [[nodiscard]] Core::f32 GetAspectRatio() const { return mAspectRatio; }

        [[nodiscard]] Core::f32 GetMoveSpeed() const { return mMoveSpeed; }
        [[nodiscard]] Core::f32 GetMouseSensitivity() const { return mMouseSensitivity; }

        [[nodiscard]] Core::Vector3 GetForwardVector() const;
        [[nodiscard]] Core::Vector3 GetRightVector() const;
        [[nodiscard]] Core::Vector3 GetUpVector() const;

        [[nodiscard]] const Core::Matrix4& GetViewMatrix() const { return mViewMatrix; }
        [[nodiscard]] const Core::Matrix4& GetProjectionMatrix() const { return mProjectionMatrix; }

    private:
        void UpdateMatrices();
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();

        static Core::f32 SanitizeFieldOfViewY(Core::f32 fieldOfViewYDegrees);
        static Core::f32 SanitizeNearPlane(Core::f32 nearPlane);
        static Core::f32 SanitizeFarPlane(Core::f32 nearPlane, Core::f32 farPlane);
        static Core::f32 SanitizeAspectRatio(Core::f32 aspectRatio);
        static Core::f32 WrapDegrees(Core::f32 degrees);

    private:
        bool mInitialized = false;

        Core::Vector3 mPosition = Core::Vector3(0.0f, 1.6f, -5.0f);
        Core::Rotator mRotation = Core::Rotator::Zero();

        Core::f32 mFieldOfViewYDegrees = 75.0f;
        Core::f32 mNearPlane = 0.05f;
        Core::f32 mFarPlane = 10000.0f;
        Core::f32 mAspectRatio = 16.0f / 9.0f;

        Core::f32 mMoveSpeed = 7.5f;
        Core::f32 mMouseSensitivity = 0.08f;

        Core::Matrix4 mViewMatrix = Core::Matrix4::Identity();
        Core::Matrix4 mProjectionMatrix = Core::Matrix4::Identity();
    };
}