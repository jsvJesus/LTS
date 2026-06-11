#include "EditorPickingController.h"

#include "Engine/Camera/Camera.h"

#include "Platform/Input.h"

#include "Render/RenderSystem.h"
#include "Render/RHI/RenderTypes.h"

#include "Core/Logger.h"

#include <sstream>
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

        const Core::f32 fieldOfViewRadians =
            Core::ToRadians(mContext.MainCamera->GetFieldOfViewYDegrees());

        const Core::f32 tanHalfFov =
            static_cast<Core::f32>(std::tan(fieldOfViewRadians * 0.5f));

        const Core::f32 aspectRatio = mContext.MainCamera->GetAspectRatio();

        const Core::Vector3 forward = mContext.MainCamera->GetForwardVector().Normalized();
        const Core::Vector3 right = mContext.MainCamera->GetRightVector().Normalized();
        const Core::Vector3 up = mContext.MainCamera->GetUpVector().Normalized();

        Core::Vector3 direction =
            forward +
            right * (normalizedX * aspectRatio * tanHalfFov) +
            up * (normalizedY * tanHalfFov);

        direction.Normalize();

        if (direction.LengthSquared() <= 0.00001f)
            return false;

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