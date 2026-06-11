#include "EditorPickingController.h"

#include "Engine/Camera/Camera.h"

#include "Platform/Input.h"

#include "Render/RenderSystem.h"
#include "Render/RHI/RenderTypes.h"

#include "Core/Logger.h"

#include <sstream>
#include <algorithm>
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

        Core::f32 BuildNormalizedMouseCoordinateX(
            const Core::i32 mouseX,
            const Core::u32 viewportWidth
        )
        {
            if (viewportWidth == 0)
                return 0.0f;

            const Core::f32 width = static_cast<Core::f32>(viewportWidth);
            return (static_cast<Core::f32>(mouseX) / width) * 2.0f - 1.0f;
        }

        Core::f32 BuildNormalizedMouseCoordinateY(
            const Core::i32 mouseY,
            const Core::u32 viewportHeight
        )
        {
            if (viewportHeight == 0)
                return 0.0f;

            const Core::f32 height = static_cast<Core::f32>(viewportHeight);
            return 1.0f - (static_cast<Core::f32>(mouseY) / height) * 2.0f;
        }

        bool TryInvertMatrix4(
            const Core::Matrix4& matrix,
            Core::Matrix4& outInverse
        )
        {
            Core::f32 augmented[4][8] {};

            for (Core::usize row = 0; row < 4; ++row)
            {
                for (Core::usize column = 0; column < 4; ++column)
                {
                    augmented[row][column] = matrix.M[row][column];
                }

                augmented[row][row + 4] = 1.0f;
            }

            for (Core::usize pivot = 0; pivot < 4; ++pivot)
            {
                Core::usize bestRow = pivot;
                Core::f32 bestValue = static_cast<Core::f32>(
                    std::fabs(augmented[pivot][pivot])
                );

                for (Core::usize row = pivot + 1; row < 4; ++row)
                {
                    const Core::f32 value = static_cast<Core::f32>(
                        std::fabs(augmented[row][pivot])
                    );

                    if (value > bestValue)
                    {
                        bestValue = value;
                        bestRow = row;
                    }
                }

                if (bestValue <= 0.00001f)
                    return false;

                if (bestRow != pivot)
                {
                    for (Core::usize column = 0; column < 8; ++column)
                    {
                        std::swap(augmented[pivot][column], augmented[bestRow][column]);
                    }
                }

                const Core::f32 pivotValue = augmented[pivot][pivot];

                for (Core::usize column = 0; column < 8; ++column)
                {
                    augmented[pivot][column] /= pivotValue;
                }

                for (Core::usize row = 0; row < 4; ++row)
                {
                    if (row == pivot)
                        continue;

                    const Core::f32 factor = augmented[row][pivot];

                    if (std::fabs(factor) <= 0.00001f)
                        continue;

                    for (Core::usize column = 0; column < 8; ++column)
                    {
                        augmented[row][column] -= factor * augmented[pivot][column];
                    }
                }
            }

            outInverse = Core::Matrix4::Identity();

            for (Core::usize row = 0; row < 4; ++row)
            {
                for (Core::usize column = 0; column < 4; ++column)
                {
                    outInverse.M[row][column] = augmented[row][column + 4];
                }
            }

            return true;
        }
    }

    bool EditorPickingController::Initialize(
        const Engine::FApplicationRuntimeContext& context,
        const FEditorPickingControllerDesc& desc
    )
    {
        mContext = context;

        if (!mContext.InputSystem || !mContext.RenderSystem)
        {
            mInitialized = false;
            return false;
        }

        mLastPickRequest = FEditorPickRequest {};

        mDebugRayEnabled = desc.EnableDebugRay;
        mLogPickRequests = desc.LogPickRequests;
        mDebugRayLength = SanitizePositiveValue(desc.DebugRayLength, 12.0f);

        mNextRequestId = 1;
        mInitialized = true;

        Core::Logger::Info("Editor", "Editor picking controller initialized.");

        return true;
    }

    void EditorPickingController::Shutdown()
    {
        if (mInitialized)
        {
            Core::Logger::Info("Editor", "Editor picking controller shutdown.");
        }

        mContext = Engine::FApplicationRuntimeContext {};
        mLastPickRequest = FEditorPickRequest {};

        mInitialized = false;
        mDebugRayEnabled = true;
        mLogPickRequests = true;

        mDebugRayLength = 12.0f;
        mNextRequestId = 1;
    }

    void EditorPickingController::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized)
            return;

        HandleInput();
    }

    void EditorPickingController::RenderDebug()
    {
        if (!mInitialized || !mDebugRayEnabled || !mContext.RenderSystem)
            return;

        DrawLastPickRay();
    }

    void EditorPickingController::SetDebugRayEnabled(const bool enabled)
    {
        mDebugRayEnabled = enabled;
    }

    void EditorPickingController::SetPickRequestLoggingEnabled(const bool enabled)
    {
        mLogPickRequests = enabled;
    }

    bool EditorPickingController::BuildCurrentPickRay(FEditorPickRay& outRay) const
    {
        outRay = FEditorPickRay {};

        if (!mInitialized)
            return false;

        return BuildPickRay(outRay);
    }

    void EditorPickingController::HandleInput()
    {
        if (!mContext.InputSystem)
            return;

        if (!mContext.InputSystem->IsMouseButtonPressed(Platform::MouseButton::Left))
            return;

        FEditorPickRequest request {};

        if (!BuildPickRequest(request))
        {
            Core::Logger::Warning("Editor", "Editor pick request ignored. Picking context is incomplete.");
            return;
        }

        mLastPickRequest = request;

        if (mLogPickRequests)
        {
            LogPickRequest(mLastPickRequest);
        }
    }

    bool EditorPickingController::BuildPickRequest(FEditorPickRequest& outRequest)
    {
        if (!mContext.InputSystem || !mContext.RenderSystem)
            return false;

        FEditorPickRay ray {};

        if (!BuildPickRay(ray))
            return false;

        outRequest = FEditorPickRequest {};

        outRequest.RequestId = mNextRequestId++;
        outRequest.MouseX = mContext.InputSystem->GetMouseX();
        outRequest.MouseY = mContext.InputSystem->GetMouseY();

        outRequest.ViewportWidth = static_cast<Core::u32>(mContext.RenderSystem->GetWidth());
        outRequest.ViewportHeight = static_cast<Core::u32>(mContext.RenderSystem->GetHeight());

        outRequest.NormalizedX = BuildNormalizedMouseCoordinateX(
            outRequest.MouseX,
            outRequest.ViewportWidth
        );

        outRequest.NormalizedY = BuildNormalizedMouseCoordinateY(
            outRequest.MouseY,
            outRequest.ViewportHeight
        );

        outRequest.Ray = ray;

        return outRequest.IsValid();
    }

    bool EditorPickingController::BuildPickRay(FEditorPickRay& outRay) const
    {
        outRay = FEditorPickRay {};

        if (!mContext.MainCamera || !mContext.InputSystem || !mContext.RenderSystem)
            return false;

        if (mContext.InputSystem->IsCursorLocked())
        {
            Core::Vector3 direction =
                mContext.MainCamera->GetForwardVector().Normalized();

            if (direction.LengthSquared() <= 0.00001f)
                return false;

            outRay.Origin = mContext.MainCamera->GetPosition();
            outRay.Direction = direction;

            return outRay.IsValid();
        }

        const Core::u32 viewportWidth =
            static_cast<Core::u32>(mContext.RenderSystem->GetWidth());

        const Core::u32 viewportHeight =
            static_cast<Core::u32>(mContext.RenderSystem->GetHeight());

        if (viewportWidth == 0 || viewportHeight == 0)
            return false;

        const Core::f32 normalizedX = BuildNormalizedMouseCoordinateX(
            mContext.InputSystem->GetMouseX(),
            viewportWidth
        );

        const Core::f32 normalizedY = BuildNormalizedMouseCoordinateY(
            mContext.InputSystem->GetMouseY(),
            viewportHeight
        );

        const Core::Matrix4 viewProjection =
            mContext.MainCamera->GetViewMatrix() *
            mContext.MainCamera->GetProjectionMatrix();

        Core::Matrix4 inverseViewProjection {};

        if (!TryInvertMatrix4(viewProjection, inverseViewProjection))
            return false;

        const Core::Vector3 nearPoint = Core::Matrix4::TransformPoint(
            Core::Vector3(normalizedX, normalizedY, 0.0f),
            inverseViewProjection
        );

        const Core::Vector3 farPoint = Core::Matrix4::TransformPoint(
            Core::Vector3(normalizedX, normalizedY, 1.0f),
            inverseViewProjection
        );

        Core::Vector3 direction = farPoint - nearPoint;

        if (direction.LengthSquared() <= 0.00001f)
            return false;

        direction.Normalize();

        outRay.Origin = mContext.MainCamera->GetPosition();
        outRay.Direction = direction;

        return outRay.IsValid();
    }

    void EditorPickingController::LogPickRequest(const FEditorPickRequest& request) const
    {
        std::ostringstream stream;

        stream << "Editor pick request: "
               << "Id="
               << request.RequestId
               << ", Mouse=("
               << request.MouseX
               << ", "
               << request.MouseY
               << ")"
               << ", Viewport=("
               << request.ViewportWidth
               << "x"
               << request.ViewportHeight
               << ")"
               << ", Normalized=("
               << request.NormalizedX
               << ", "
               << request.NormalizedY
               << ")"
               << ", RayOrigin=("
               << request.Ray.Origin.X
               << ", "
               << request.Ray.Origin.Y
               << ", "
               << request.Ray.Origin.Z
               << ")"
               << ", RayDirection=("
               << request.Ray.Direction.X
               << ", "
               << request.Ray.Direction.Y
               << ", "
               << request.Ray.Direction.Z
               << ").";

        Core::Logger::Info("Editor", stream.str());
    }

    void EditorPickingController::DrawLastPickRay()
    {
        if (!mContext.RenderSystem || !mLastPickRequest.IsValid())
            return;

        const Render::FRenderColor rayColor =
            MakeColor(0.10f, 0.95f, 1.00f, 1.0f);

        const Render::FRenderColor endColor =
            MakeColor(1.00f, 0.35f, 0.95f, 1.0f);

        const Core::Vector3 start = mLastPickRequest.Ray.Origin;
        const Core::Vector3 end =
            start + mLastPickRequest.Ray.Direction * GetSafeDebugRayLength();

        mContext.RenderSystem->DrawDebugLine(start, end, rayColor);

        const Core::f32 markerSize = 0.15f;

        mContext.RenderSystem->DrawDebugLine(
            end - Core::Vector3::Right() * markerSize,
            end + Core::Vector3::Right() * markerSize,
            endColor
        );

        mContext.RenderSystem->DrawDebugLine(
            end - Core::Vector3::Up() * markerSize,
            end + Core::Vector3::Up() * markerSize,
            endColor
        );

        mContext.RenderSystem->DrawDebugLine(
            end - Core::Vector3::Forward() * markerSize,
            end + Core::Vector3::Forward() * markerSize,
            endColor
        );
    }

    Core::f32 EditorPickingController::GetSafeDebugRayLength() const
    {
        return SanitizePositiveValue(mDebugRayLength, 12.0f);
    }
}