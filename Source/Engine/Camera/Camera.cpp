#include "Camera.h"

#include "Platform/Input.h"

#include <cmath>

namespace Engine
{
    namespace
    {
        constexpr Core::f32 MinPitchDegrees = -89.0f;
        constexpr Core::f32 MaxPitchDegrees = 89.0f;

        constexpr Core::f32 MinMoveSpeed = 0.0f;
        constexpr Core::f32 MaxMoveSpeed = 10000.0f;

        constexpr Core::f32 MinMouseSensitivity = 0.0f;
        constexpr Core::f32 MaxMouseSensitivity = 10.0f;
    }

    bool Camera::Initialize(const FCameraDesc& desc)
    {
        mPosition = desc.Position;
        mRotation = desc.Rotation;
        mRotation.ClampPitch(MinPitchDegrees, MaxPitchDegrees);
        mRotation.Yaw = WrapDegrees(mRotation.Yaw);
        mRotation.Roll = WrapDegrees(mRotation.Roll);

        mFieldOfViewYDegrees = SanitizeFieldOfViewY(desc.FieldOfViewYDegrees);
        mNearPlane = SanitizeNearPlane(desc.NearPlane);
        mFarPlane = SanitizeFarPlane(mNearPlane, desc.FarPlane);
        mAspectRatio = SanitizeAspectRatio(desc.AspectRatio);

        mMoveSpeed = Core::Clamp(desc.MoveSpeed, MinMoveSpeed, MaxMoveSpeed);
        mMouseSensitivity = Core::Clamp(desc.MouseSensitivity, MinMouseSensitivity, MaxMouseSensitivity);

        UpdateMatrices();

        mInitialized = true;
        return true;
    }

    void Camera::Reset()
    {
        FCameraDesc desc {};
        Initialize(desc);
    }

    void Camera::UpdateFromInput(const Platform::InputSystem& inputSystem, const double deltaSeconds)
    {
        if (!mInitialized)
            return;

        const Core::f32 safeDeltaSeconds = Core::Clamp(
            static_cast<Core::f32>(deltaSeconds),
            0.0f,
            0.25f
        );

        bool changed = false;

        if (inputSystem.IsCursorLocked())
        {
            const Core::i32 rawMouseDeltaX = inputSystem.GetRawMouseDeltaX();
            const Core::i32 rawMouseDeltaY = inputSystem.GetRawMouseDeltaY();

            if (rawMouseDeltaX != 0 || rawMouseDeltaY != 0)
            {
                mRotation.Yaw += static_cast<Core::f32>(rawMouseDeltaX) * mMouseSensitivity;
                mRotation.Pitch += static_cast<Core::f32>(rawMouseDeltaY) * mMouseSensitivity;

                mRotation.Yaw = WrapDegrees(mRotation.Yaw);
                mRotation.ClampPitch(MinPitchDegrees, MaxPitchDegrees);

                changed = true;
            }
        }

        Core::Vector3 movement = Core::Vector3::Zero();

        if (inputSystem.IsKeyDown(Platform::KeyCode::W))
            movement += GetForwardVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::S))
            movement -= GetForwardVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::D))
            movement += GetRightVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::A))
            movement -= GetRightVector();

        if (inputSystem.IsKeyDown(Platform::KeyCode::Space))
            movement += Core::Vector3::Up();

        if (inputSystem.IsKeyDown(Platform::KeyCode::LeftControl))
            movement -= Core::Vector3::Up();

        if (movement.LengthSquared() > 0.00001f)
        {
            movement.Normalize();

            mPosition += movement * (mMoveSpeed * safeDeltaSeconds);
            changed = true;
        }

        if (changed)
        {
            UpdateMatrices();
        }
    }

    void Camera::SetPosition(const Core::Vector3& position)
    {
        mPosition = position;
        UpdateViewMatrix();
    }

    void Camera::SetRotation(const Core::Rotator& rotation)
    {
        mRotation = rotation;
        mRotation.ClampPitch(MinPitchDegrees, MaxPitchDegrees);
        mRotation.Yaw = WrapDegrees(mRotation.Yaw);
        mRotation.Roll = WrapDegrees(mRotation.Roll);

        UpdateViewMatrix();
    }

    void Camera::SetFieldOfViewYDegrees(const Core::f32 fieldOfViewYDegrees)
    {
        mFieldOfViewYDegrees = SanitizeFieldOfViewY(fieldOfViewYDegrees);
        UpdateProjectionMatrix();
    }

    void Camera::SetClippingPlanes(const Core::f32 nearPlane, const Core::f32 farPlane)
    {
        mNearPlane = SanitizeNearPlane(nearPlane);
        mFarPlane = SanitizeFarPlane(mNearPlane, farPlane);

        UpdateProjectionMatrix();
    }

    void Camera::SetAspectRatio(const Core::f32 aspectRatio)
    {
        mAspectRatio = SanitizeAspectRatio(aspectRatio);
        UpdateProjectionMatrix();
    }

    void Camera::SetMoveSpeed(const Core::f32 moveSpeed)
    {
        mMoveSpeed = Core::Clamp(moveSpeed, MinMoveSpeed, MaxMoveSpeed);
    }

    void Camera::SetMouseSensitivity(const Core::f32 mouseSensitivity)
    {
        mMouseSensitivity = Core::Clamp(mouseSensitivity, MinMouseSensitivity, MaxMouseSensitivity);
    }

    Core::Vector3 Camera::GetForwardVector() const
    {
        return mRotation.GetForwardVector();
    }

    Core::Vector3 Camera::GetRightVector() const
    {
        return mRotation.GetRightVector();
    }

    Core::Vector3 Camera::GetUpVector() const
    {
        return mRotation.GetUpVector();
    }

    void Camera::UpdateMatrices()
    {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::UpdateViewMatrix()
    {
        const Core::Vector3 forward = GetForwardVector();
        const Core::Vector3 target = mPosition + forward;

        mViewMatrix = Core::Matrix4::CreateLookAtLH(
            mPosition,
            target,
            Core::Vector3::Up()
        );
    }

    void Camera::UpdateProjectionMatrix()
    {
        mProjectionMatrix = Core::Matrix4::CreatePerspectiveFovLH(
            Core::ToRadians(mFieldOfViewYDegrees),
            mAspectRatio,
            mNearPlane,
            mFarPlane
        );
    }

    Core::f32 Camera::SanitizeFieldOfViewY(const Core::f32 fieldOfViewYDegrees)
    {
        return Core::Clamp(fieldOfViewYDegrees, 1.0f, 170.0f);
    }

    Core::f32 Camera::SanitizeNearPlane(const Core::f32 nearPlane)
    {
        return nearPlane > 0.00001f ? nearPlane : 0.00001f;
    }

    Core::f32 Camera::SanitizeFarPlane(const Core::f32 nearPlane, const Core::f32 farPlane)
    {
        return farPlane > nearPlane + 0.00001f
            ? farPlane
            : nearPlane + 1000.0f;
    }

    Core::f32 Camera::SanitizeAspectRatio(const Core::f32 aspectRatio)
    {
        return std::fabs(aspectRatio) > 0.00001f ? aspectRatio : 1.0f;
    }

    Core::f32 Camera::WrapDegrees(Core::f32 degrees)
    {
        while (degrees > 180.0f)
        {
            degrees -= 360.0f;
        }

        while (degrees < -180.0f)
        {
            degrees += 360.0f;
        }

        return degrees;
    }
}