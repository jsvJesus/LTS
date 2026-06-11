#pragma once

#include "Core/BaseTypes.h"

#include <algorithm>
#include <cmath>

namespace Core
{
    constexpr f32 Pi = 3.14159265358979323846f;
    constexpr f32 TwoPi = Pi * 2.0f;
    constexpr f32 HalfPi = Pi * 0.5f;
    constexpr f32 DegToRad = Pi / 180.0f;
    constexpr f32 RadToDeg = 180.0f / Pi;

    [[nodiscard]] inline f32 ToRadians(const f32 degrees)
    {
        return degrees * DegToRad;
    }

    [[nodiscard]] inline f32 ToDegrees(const f32 radians)
    {
        return radians * RadToDeg;
    }

    [[nodiscard]] inline f32 Clamp(const f32 value, const f32 minValue, const f32 maxValue)
    {
        return std::clamp(value, minValue, maxValue);
    }

    [[nodiscard]] inline bool NearlyEqual(
        const f32 a,
        const f32 b,
        const f32 epsilon = 0.00001f
    )
    {
        return std::fabs(a - b) <= epsilon;
    }

    [[nodiscard]] inline f32 SafeSqrt(const f32 value)
    {
        return value > 0.0f ? std::sqrt(value) : 0.0f;
    }

    struct Vector2 final
    {
        f32 X = 0.0f;
        f32 Y = 0.0f;

        constexpr Vector2() = default;

        constexpr Vector2(const f32 x, const f32 y)
            : X(x)
            , Y(y)
        {
        }

        [[nodiscard]] static constexpr Vector2 Zero()
        {
            return Vector2(0.0f, 0.0f);
        }

        [[nodiscard]] static constexpr Vector2 One()
        {
            return Vector2(1.0f, 1.0f);
        }

        [[nodiscard]] f32 LengthSquared() const
        {
            return X * X + Y * Y;
        }

        [[nodiscard]] f32 Length() const
        {
            return SafeSqrt(LengthSquared());
        }

        [[nodiscard]] Vector2 Normalized() const
        {
            const f32 length = Length();

            if (length <= 0.00001f)
                return Zero();

            const f32 invLength = 1.0f / length;
            return Vector2(X * invLength, Y * invLength);
        }

        void Normalize()
        {
            *this = Normalized();
        }

        [[nodiscard]] static f32 Dot(const Vector2& a, const Vector2& b)
        {
            return a.X * b.X + a.Y * b.Y;
        }

        [[nodiscard]] Vector2 operator+() const
        {
            return *this;
        }

        [[nodiscard]] Vector2 operator-() const
        {
            return Vector2(-X, -Y);
        }

        [[nodiscard]] Vector2 operator+(const Vector2& rhs) const
        {
            return Vector2(X + rhs.X, Y + rhs.Y);
        }

        [[nodiscard]] Vector2 operator-(const Vector2& rhs) const
        {
            return Vector2(X - rhs.X, Y - rhs.Y);
        }

        [[nodiscard]] Vector2 operator*(const f32 scalar) const
        {
            return Vector2(X * scalar, Y * scalar);
        }

        [[nodiscard]] Vector2 operator/(const f32 scalar) const
        {
            if (NearlyEqual(scalar, 0.0f))
                return Zero();

            const f32 invScalar = 1.0f / scalar;
            return Vector2(X * invScalar, Y * invScalar);
        }

        Vector2& operator+=(const Vector2& rhs)
        {
            X += rhs.X;
            Y += rhs.Y;
            return *this;
        }

        Vector2& operator-=(const Vector2& rhs)
        {
            X -= rhs.X;
            Y -= rhs.Y;
            return *this;
        }

        Vector2& operator*=(const f32 scalar)
        {
            X *= scalar;
            Y *= scalar;
            return *this;
        }

        Vector2& operator/=(const f32 scalar)
        {
            *this = *this / scalar;
            return *this;
        }
    };

    [[nodiscard]] inline Vector2 operator*(const f32 scalar, const Vector2& vector)
    {
        return vector * scalar;
    }

    struct Vector3 final
    {
        f32 X = 0.0f;
        f32 Y = 0.0f;
        f32 Z = 0.0f;

        constexpr Vector3() = default;

        constexpr Vector3(const f32 x, const f32 y, const f32 z)
            : X(x)
            , Y(y)
            , Z(z)
        {
        }

        [[nodiscard]] static constexpr Vector3 Zero()
        {
            return Vector3(0.0f, 0.0f, 0.0f);
        }

        [[nodiscard]] static constexpr Vector3 One()
        {
            return Vector3(1.0f, 1.0f, 1.0f);
        }

        [[nodiscard]] static constexpr Vector3 Right()
        {
            return Vector3(1.0f, 0.0f, 0.0f);
        }

        [[nodiscard]] static constexpr Vector3 Up()
        {
            return Vector3(0.0f, 1.0f, 0.0f);
        }

        [[nodiscard]] static constexpr Vector3 Forward()
        {
            return Vector3(0.0f, 0.0f, 1.0f);
        }

        [[nodiscard]] f32 LengthSquared() const
        {
            return X * X + Y * Y + Z * Z;
        }

        [[nodiscard]] f32 Length() const
        {
            return SafeSqrt(LengthSquared());
        }

        [[nodiscard]] Vector3 Normalized() const
        {
            const f32 length = Length();

            if (length <= 0.00001f)
                return Zero();

            const f32 invLength = 1.0f / length;
            return Vector3(X * invLength, Y * invLength, Z * invLength);
        }

        void Normalize()
        {
            *this = Normalized();
        }

        [[nodiscard]] static f32 Dot(const Vector3& a, const Vector3& b)
        {
            return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
        }

        [[nodiscard]] static Vector3 Cross(const Vector3& a, const Vector3& b)
        {
            return Vector3(
                a.Y * b.Z - a.Z * b.Y,
                a.Z * b.X - a.X * b.Z,
                a.X * b.Y - a.Y * b.X
            );
        }

        [[nodiscard]] Vector3 operator+() const
        {
            return *this;
        }

        [[nodiscard]] Vector3 operator-() const
        {
            return Vector3(-X, -Y, -Z);
        }

        [[nodiscard]] Vector3 operator+(const Vector3& rhs) const
        {
            return Vector3(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
        }

        [[nodiscard]] Vector3 operator-(const Vector3& rhs) const
        {
            return Vector3(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
        }

        [[nodiscard]] Vector3 operator*(const f32 scalar) const
        {
            return Vector3(X * scalar, Y * scalar, Z * scalar);
        }

        [[nodiscard]] Vector3 operator/(const f32 scalar) const
        {
            if (NearlyEqual(scalar, 0.0f))
                return Zero();

            const f32 invScalar = 1.0f / scalar;
            return Vector3(X * invScalar, Y * invScalar, Z * invScalar);
        }

        Vector3& operator+=(const Vector3& rhs)
        {
            X += rhs.X;
            Y += rhs.Y;
            Z += rhs.Z;
            return *this;
        }

        Vector3& operator-=(const Vector3& rhs)
        {
            X -= rhs.X;
            Y -= rhs.Y;
            Z -= rhs.Z;
            return *this;
        }

        Vector3& operator*=(const f32 scalar)
        {
            X *= scalar;
            Y *= scalar;
            Z *= scalar;
            return *this;
        }

        Vector3& operator/=(const f32 scalar)
        {
            *this = *this / scalar;
            return *this;
        }
    };

    [[nodiscard]] inline Vector3 operator*(const f32 scalar, const Vector3& vector)
    {
        return vector * scalar;
    }

    struct Quaternion final
    {
        f32 X = 0.0f;
        f32 Y = 0.0f;
        f32 Z = 0.0f;
        f32 W = 1.0f;

        constexpr Quaternion() = default;

        constexpr Quaternion(const f32 x, const f32 y, const f32 z, const f32 w)
            : X(x)
            , Y(y)
            , Z(z)
            , W(w)
        {
        }

        [[nodiscard]] static constexpr Quaternion Identity()
        {
            return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
        }

        [[nodiscard]] f32 LengthSquared() const
        {
            return X * X + Y * Y + Z * Z + W * W;
        }

        [[nodiscard]] f32 Length() const
        {
            return SafeSqrt(LengthSquared());
        }

        [[nodiscard]] Quaternion Normalized() const
        {
            const f32 length = Length();

            if (length <= 0.00001f)
                return Identity();

            const f32 invLength = 1.0f / length;

            return Quaternion(
                X * invLength,
                Y * invLength,
                Z * invLength,
                W * invLength
            );
        }

        [[nodiscard]] Quaternion Conjugated() const
        {
            return Quaternion(-X, -Y, -Z, W);
        }

        [[nodiscard]] Quaternion operator*(const Quaternion& rhs) const
        {
            return Quaternion(
                W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y,
                W * rhs.Y - X * rhs.Z + Y * rhs.W + Z * rhs.X,
                W * rhs.Z + X * rhs.Y - Y * rhs.X + Z * rhs.W,
                W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z
            );
        }

        [[nodiscard]] Vector3 RotateVector(const Vector3& vector) const
        {
            const Quaternion normalized = Normalized();
            const Quaternion input(vector.X, vector.Y, vector.Z, 0.0f);
            const Quaternion result = normalized * input * normalized.Conjugated();

            return Vector3(result.X, result.Y, result.Z);
        }

        [[nodiscard]] static Quaternion FromAxisAngleRadians(
            const Vector3& axis,
            const f32 angleRadians
        )
        {
            const Vector3 normalizedAxis = axis.Normalized();

            if (normalizedAxis.LengthSquared() <= 0.00001f)
                return Identity();

            const f32 halfAngle = angleRadians * 0.5f;
            const f32 sine = std::sin(halfAngle);
            const f32 cosine = std::cos(halfAngle);

            return Quaternion(
                normalizedAxis.X * sine,
                normalizedAxis.Y * sine,
                normalizedAxis.Z * sine,
                cosine
            ).Normalized();
        }

        [[nodiscard]] static Quaternion FromEulerDegrees(
            const f32 pitchDegrees,
            const f32 yawDegrees,
            const f32 rollDegrees
        )
        {
            const Quaternion pitch = FromAxisAngleRadians(Vector3::Right(), ToRadians(pitchDegrees));
            const Quaternion yaw = FromAxisAngleRadians(Vector3::Up(), ToRadians(yawDegrees));
            const Quaternion roll = FromAxisAngleRadians(Vector3::Forward(), ToRadians(rollDegrees));

            return (yaw * pitch * roll).Normalized();
        }
    };

    struct Matrix4 final
    {
        f32 M[4][4] {};

        [[nodiscard]] f32* operator[](const usize row)
        {
            return M[row];
        }

        [[nodiscard]] const f32* operator[](const usize row) const
        {
            return M[row];
        }

        [[nodiscard]] static Matrix4 Zero()
        {
            return Matrix4 {};
        }

        [[nodiscard]] static Matrix4 Identity()
        {
            Matrix4 result {};

            result.M[0][0] = 1.0f;
            result.M[1][1] = 1.0f;
            result.M[2][2] = 1.0f;
            result.M[3][3] = 1.0f;

            return result;
        }

        [[nodiscard]] static Matrix4 CreateTranslation(const Vector3& translation)
        {
            Matrix4 result = Identity();

            result.M[3][0] = translation.X;
            result.M[3][1] = translation.Y;
            result.M[3][2] = translation.Z;

            return result;
        }

        [[nodiscard]] static Matrix4 CreateScale(const Vector3& scale)
        {
            Matrix4 result {};

            result.M[0][0] = scale.X;
            result.M[1][1] = scale.Y;
            result.M[2][2] = scale.Z;
            result.M[3][3] = 1.0f;

            return result;
        }

        [[nodiscard]] static Matrix4 CreateRotationX(const f32 angleRadians)
        {
            Matrix4 result = Identity();

            const f32 sine = std::sin(angleRadians);
            const f32 cosine = std::cos(angleRadians);

            result.M[1][1] = cosine;
            result.M[1][2] = sine;
            result.M[2][1] = -sine;
            result.M[2][2] = cosine;

            return result;
        }

        [[nodiscard]] static Matrix4 CreateRotationY(const f32 angleRadians)
        {
            Matrix4 result = Identity();

            const f32 sine = std::sin(angleRadians);
            const f32 cosine = std::cos(angleRadians);

            result.M[0][0] = cosine;
            result.M[0][2] = -sine;
            result.M[2][0] = sine;
            result.M[2][2] = cosine;

            return result;
        }

        [[nodiscard]] static Matrix4 CreateRotationZ(const f32 angleRadians)
        {
            Matrix4 result = Identity();

            const f32 sine = std::sin(angleRadians);
            const f32 cosine = std::cos(angleRadians);

            result.M[0][0] = cosine;
            result.M[0][1] = sine;
            result.M[1][0] = -sine;
            result.M[1][1] = cosine;

            return result;
        }

        [[nodiscard]] static Matrix4 CreateRotationYawPitchRollDegrees(
            const f32 yawDegrees,
            const f32 pitchDegrees,
            const f32 rollDegrees
        )
        {
            const Matrix4 yaw = CreateRotationY(ToRadians(yawDegrees));
            const Matrix4 pitch = CreateRotationX(ToRadians(pitchDegrees));
            const Matrix4 roll = CreateRotationZ(ToRadians(rollDegrees));

            return yaw * pitch * roll;
        }

        [[nodiscard]] static Matrix4 CreatePerspectiveFovLH(
            const f32 fieldOfViewYRadians,
            const f32 aspectRatio,
            const f32 nearPlane,
            const f32 farPlane
        )
        {
            Matrix4 result {};

            const f32 safeAspectRatio =
                std::fabs(aspectRatio) > 0.00001f ? aspectRatio : 1.0f;

            const f32 safeNearPlane = nearPlane > 0.00001f ? nearPlane : 0.00001f;
            const f32 safeFarPlane = farPlane > safeNearPlane + 0.00001f
                ? farPlane
                : safeNearPlane + 1000.0f;

            const f32 yScale = 1.0f / std::tan(fieldOfViewYRadians * 0.5f);
            const f32 xScale = yScale / safeAspectRatio;

            result.M[0][0] = xScale;
            result.M[1][1] = yScale;
            result.M[2][2] = safeFarPlane / (safeFarPlane - safeNearPlane);
            result.M[2][3] = 1.0f;
            result.M[3][2] = (-safeNearPlane * safeFarPlane) / (safeFarPlane - safeNearPlane);

            return result;
        }

        [[nodiscard]] static Matrix4 CreateLookAtLH(
            const Vector3& eye,
            const Vector3& target,
            const Vector3& up
        )
        {
            const Vector3 zAxis = (target - eye).Normalized();
            const Vector3 xAxis = Vector3::Cross(up, zAxis).Normalized();
            const Vector3 yAxis = Vector3::Cross(zAxis, xAxis);

            Matrix4 result = Identity();

            result.M[0][0] = xAxis.X;
            result.M[0][1] = yAxis.X;
            result.M[0][2] = zAxis.X;
            result.M[0][3] = 0.0f;

            result.M[1][0] = xAxis.Y;
            result.M[1][1] = yAxis.Y;
            result.M[1][2] = zAxis.Y;
            result.M[1][3] = 0.0f;

            result.M[2][0] = xAxis.Z;
            result.M[2][1] = yAxis.Z;
            result.M[2][2] = zAxis.Z;
            result.M[2][3] = 0.0f;

            result.M[3][0] = -Vector3::Dot(xAxis, eye);
            result.M[3][1] = -Vector3::Dot(yAxis, eye);
            result.M[3][2] = -Vector3::Dot(zAxis, eye);
            result.M[3][3] = 1.0f;

            return result;
        }

        [[nodiscard]] Matrix4 operator*(const Matrix4& rhs) const
        {
            Matrix4 result {};

            for (usize row = 0; row < 4; ++row)
            {
                for (usize column = 0; column < 4; ++column)
                {
                    result.M[row][column] =
                        M[row][0] * rhs.M[0][column] +
                        M[row][1] * rhs.M[1][column] +
                        M[row][2] * rhs.M[2][column] +
                        M[row][3] * rhs.M[3][column];
                }
            }

            return result;
        }

        [[nodiscard]] static Vector3 TransformPoint(
            const Vector3& point,
            const Matrix4& matrix
        )
        {
            const f32 x =
                point.X * matrix.M[0][0] +
                point.Y * matrix.M[1][0] +
                point.Z * matrix.M[2][0] +
                matrix.M[3][0];

            const f32 y =
                point.X * matrix.M[0][1] +
                point.Y * matrix.M[1][1] +
                point.Z * matrix.M[2][1] +
                matrix.M[3][1];

            const f32 z =
                point.X * matrix.M[0][2] +
                point.Y * matrix.M[1][2] +
                point.Z * matrix.M[2][2] +
                matrix.M[3][2];

            const f32 w =
                point.X * matrix.M[0][3] +
                point.Y * matrix.M[1][3] +
                point.Z * matrix.M[2][3] +
                matrix.M[3][3];

            if (std::fabs(w) <= 0.00001f)
                return Vector3(x, y, z);

            const f32 invW = 1.0f / w;
            return Vector3(x * invW, y * invW, z * invW);
        }

        [[nodiscard]] static Vector3 TransformVector(
            const Vector3& vector,
            const Matrix4& matrix
        )
        {
            const f32 x =
                vector.X * matrix.M[0][0] +
                vector.Y * matrix.M[1][0] +
                vector.Z * matrix.M[2][0];

            const f32 y =
                vector.X * matrix.M[0][1] +
                vector.Y * matrix.M[1][1] +
                vector.Z * matrix.M[2][1];

            const f32 z =
                vector.X * matrix.M[0][2] +
                vector.Y * matrix.M[1][2] +
                vector.Z * matrix.M[2][2];

            return Vector3(x, y, z);
        }
    };

    struct Rotator final
    {
        f32 Pitch = 0.0f;
        f32 Yaw = 0.0f;
        f32 Roll = 0.0f;

        constexpr Rotator() = default;

        constexpr Rotator(const f32 pitch, const f32 yaw, const f32 roll)
            : Pitch(pitch)
            , Yaw(yaw)
            , Roll(roll)
        {
        }

        [[nodiscard]] static constexpr Rotator Zero()
        {
            return Rotator(0.0f, 0.0f, 0.0f);
        }

        [[nodiscard]] Matrix4 ToMatrix() const
        {
            return Matrix4::CreateRotationYawPitchRollDegrees(Yaw, Pitch, Roll);
        }

        [[nodiscard]] Quaternion ToQuaternion() const
        {
            return Quaternion::FromEulerDegrees(Pitch, Yaw, Roll);
        }

        [[nodiscard]] Vector3 GetForwardVector() const
        {
            const Matrix4 rotation = ToMatrix();
            return Matrix4::TransformVector(Vector3::Forward(), rotation).Normalized();
        }

        [[nodiscard]] Vector3 GetRightVector() const
        {
            const Matrix4 rotation = ToMatrix();
            return Matrix4::TransformVector(Vector3::Right(), rotation).Normalized();
        }

        [[nodiscard]] Vector3 GetUpVector() const
        {
            const Matrix4 rotation = ToMatrix();
            return Matrix4::TransformVector(Vector3::Up(), rotation).Normalized();
        }

        void ClampPitch(const f32 minPitchDegrees, const f32 maxPitchDegrees)
        {
            Pitch = Clamp(Pitch, minPitchDegrees, maxPitchDegrees);
        }
    };
}

static_assert(sizeof(Core::Vector2) == sizeof(Core::f32) * 2);
static_assert(sizeof(Core::Vector3) == sizeof(Core::f32) * 3);
static_assert(sizeof(Core::Quaternion) == sizeof(Core::f32) * 4);
static_assert(sizeof(Core::Matrix4) == sizeof(Core::f32) * 16);