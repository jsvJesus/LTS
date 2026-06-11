#include "EditorToolModeController.h"

#include "Engine/Camera/Camera.h"

#include "Platform/Input.h"

#include "Render/RenderSystem.h"
#include "Render/RHI/RenderTypes.h"

#include "Core/Logger.h"

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

        Core::String BuildToolModeLogMessage(
            const Core::StringView prefix,
            const EEditorToolMode mode
        )
        {
            Core::String message;
            message.reserve(prefix.size() + 32);

            message.append(prefix.data(), prefix.size());

            const Core::StringView modeName = GetEditorToolModeName(mode);
            message.append(modeName.data(), modeName.size());

            message += ".";

            return message;
        }
    }

    bool EditorToolModeController::Initialize(
        const Engine::FApplicationRuntimeContext& context,
        const FEditorToolModeControllerDesc& desc
    )
    {
        mContext = context;

        if (!mContext.RenderSystem)
        {
            mInitialized = false;
            return false;
        }

        mCurrentToolMode = desc.InitialToolMode;
        mDebugMarkerEnabled = desc.EnableDebugMarker;

        mMarkerDistance = SanitizePositiveValue(desc.MarkerDistance, 5.0f);
        mMarkerSize = SanitizePositiveValue(desc.MarkerSize, 0.75f);

        mInitialized = true;

        Core::Logger::Info(
            "Editor",
            BuildToolModeLogMessage("Editor tool mode controller initialized. CurrentMode=", mCurrentToolMode)
        );

        return true;
    }

    void EditorToolModeController::Shutdown()
    {
        if (mInitialized)
        {
            Core::Logger::Info("Editor", "Editor tool mode controller shutdown.");
        }

        mContext = Engine::FApplicationRuntimeContext {};

        mCurrentToolMode = EEditorToolMode::Select;

        mInitialized = false;
        mDebugMarkerEnabled = true;

        mMarkerDistance = 5.0f;
        mMarkerSize = 0.75f;
    }

    void EditorToolModeController::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized)
            return;

        HandleHotkeys();
    }

    void EditorToolModeController::RenderDebug()
    {
        if (!mInitialized || !mDebugMarkerEnabled || !mContext.RenderSystem)
            return;

        DrawCurrentModeMarker();
    }

    bool EditorToolModeController::SetToolMode(const EEditorToolMode mode)
    {
        if (mCurrentToolMode == mode)
            return false;

        mCurrentToolMode = mode;

        if (mInitialized)
        {
            LogToolModeChanged();
        }

        return true;
    }

    void EditorToolModeController::SetDebugMarkerEnabled(const bool enabled)
    {
        mDebugMarkerEnabled = enabled;
    }

    void EditorToolModeController::HandleHotkeys()
    {
        if (!mContext.InputSystem)
            return;

        if (mContext.InputSystem->IsKeyPressed(Platform::KeyCode::Q))
        {
            SetToolMode(EEditorToolMode::Select);
            return;
        }

        if (mContext.InputSystem->IsKeyPressed(Platform::KeyCode::W))
        {
            SetToolMode(EEditorToolMode::Move);
            return;
        }

        if (mContext.InputSystem->IsKeyPressed(Platform::KeyCode::E))
        {
            SetToolMode(EEditorToolMode::Rotate);
            return;
        }

        if (mContext.InputSystem->IsKeyPressed(Platform::KeyCode::R))
        {
            SetToolMode(EEditorToolMode::Scale);
            return;
        }
    }

    void EditorToolModeController::LogToolModeChanged() const
    {
        Core::Logger::Info(
            "Editor",
            BuildToolModeLogMessage("Editor tool mode changed: ", mCurrentToolMode)
        );
    }

    void EditorToolModeController::DrawCurrentModeMarker()
    {
        const Core::Vector3 center = GetMarkerCenter();

        switch (mCurrentToolMode)
        {
        case EEditorToolMode::Select:
            DrawSelectMarker(center, mMarkerSize);
            break;

        case EEditorToolMode::Move:
            DrawMoveMarker(center, mMarkerSize);
            break;

        case EEditorToolMode::Rotate:
            DrawRotateMarker(center, mMarkerSize);
            break;

        case EEditorToolMode::Scale:
            DrawScaleMarker(center, mMarkerSize);
            break;

        default:
            DrawSelectMarker(center, mMarkerSize);
            break;
        }
    }

    void EditorToolModeController::DrawSelectMarker(
        const Core::Vector3& center,
        const Core::f32 size
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 right = GetCameraRightVector() * size;
        const Core::Vector3 up = GetCameraUpVector() * size;

        const Render::FRenderColor mainColor =
            MakeColor(1.00f, 0.92f, 0.18f, 1.0f);

        const Render::FRenderColor softColor =
            MakeColor(0.70f, 0.60f, 0.10f, 1.0f);

        const Core::Vector3 a = center - right - up;
        const Core::Vector3 b = center + right - up;
        const Core::Vector3 c = center + right + up;
        const Core::Vector3 d = center - right + up;

        mContext.RenderSystem->DrawDebugLine(a, b, mainColor);
        mContext.RenderSystem->DrawDebugLine(b, c, mainColor);
        mContext.RenderSystem->DrawDebugLine(c, d, mainColor);
        mContext.RenderSystem->DrawDebugLine(d, a, mainColor);

        mContext.RenderSystem->DrawDebugLine(center - right, center + right, softColor);
        mContext.RenderSystem->DrawDebugLine(center - up, center + up, softColor);
    }

    void EditorToolModeController::DrawMoveMarker(
        const Core::Vector3& center,
        const Core::f32 size
    )
    {
        const Render::FRenderColor xColor =
            MakeColor(1.00f, 0.20f, 0.20f, 1.0f);

        const Render::FRenderColor yColor =
            MakeColor(0.20f, 1.00f, 0.30f, 1.0f);

        const Render::FRenderColor zColor =
            MakeColor(0.20f, 0.45f, 1.00f, 1.0f);

        const Core::f32 arrowLength = size * 1.85f;
        const Core::f32 headSize = size * 0.28f;

        DrawDebugArrow(
            center,
            center + Core::Vector3::Right() * arrowLength,
            xColor,
            headSize
        );

        DrawDebugArrow(
            center,
            center + Core::Vector3::Up() * arrowLength,
            yColor,
            headSize
        );

        DrawDebugArrow(
            center,
            center + Core::Vector3::Forward() * arrowLength,
            zColor,
            headSize
        );
    }

    void EditorToolModeController::DrawRotateMarker(
        const Core::Vector3& center,
        const Core::f32 size
    )
    {
        const Render::FRenderColor xColor =
            MakeColor(1.00f, 0.28f, 0.22f, 1.0f);

        const Render::FRenderColor yColor =
            MakeColor(0.30f, 1.00f, 0.35f, 1.0f);

        const Render::FRenderColor zColor =
            MakeColor(0.25f, 0.55f, 1.00f, 1.0f);

        const Core::f32 radius = size * 1.10f;

        DrawDebugCircle(
            center,
            Core::Vector3::Up(),
            Core::Vector3::Forward(),
            radius,
            xColor,
            32
        );

        DrawDebugCircle(
            center,
            Core::Vector3::Right(),
            Core::Vector3::Forward(),
            radius,
            yColor,
            32
        );

        DrawDebugCircle(
            center,
            Core::Vector3::Right(),
            Core::Vector3::Up(),
            radius,
            zColor,
            32
        );
    }

    void EditorToolModeController::DrawScaleMarker(
        const Core::Vector3& center,
        const Core::f32 size
    )
    {
        const Render::FRenderColor mainColor =
            MakeColor(0.85f, 0.35f, 1.00f, 1.0f);

        const Render::FRenderColor softColor =
            MakeColor(0.45f, 0.18f, 0.55f, 1.0f);

        DrawDebugBox(center, size * 0.65f, mainColor);

        if (!mContext.RenderSystem)
            return;

        const Core::f32 lineLength = size * 1.35f;

        mContext.RenderSystem->DrawDebugLine(
            center,
            center + Core::Vector3::Right() * lineLength,
            softColor
        );

        mContext.RenderSystem->DrawDebugLine(
            center,
            center + Core::Vector3::Up() * lineLength,
            softColor
        );

        mContext.RenderSystem->DrawDebugLine(
            center,
            center + Core::Vector3::Forward() * lineLength,
            softColor
        );
    }

    void EditorToolModeController::DrawDebugArrow(
        const Core::Vector3& start,
        const Core::Vector3& end,
        const Render::FRenderColor& color,
        const Core::f32 headSize
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 direction = (end - start).Normalized();

        if (direction.LengthSquared() <= 0.00001f)
            return;

        Core::Vector3 side = Core::Vector3::Cross(direction, Core::Vector3::Up());

        if (side.LengthSquared() <= 0.00001f)
        {
            side = Core::Vector3::Cross(direction, Core::Vector3::Right());
        }

        side.Normalize();

        const Core::Vector3 back = end - direction * headSize;

        mContext.RenderSystem->DrawDebugLine(start, end, color);
        mContext.RenderSystem->DrawDebugLine(end, back + side * (headSize * 0.55f), color);
        mContext.RenderSystem->DrawDebugLine(end, back - side * (headSize * 0.55f), color);
    }

    void EditorToolModeController::DrawDebugCircle(
        const Core::Vector3& center,
        const Core::Vector3& axisA,
        const Core::Vector3& axisB,
        const Core::f32 radius,
        const Render::FRenderColor& color,
        const Core::i32 segmentCount
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::i32 safeSegmentCount = segmentCount >= 3 ? segmentCount : 3;

        Core::Vector3 previousPoint =
            center + axisA.Normalized() * radius;

        for (Core::i32 segmentIndex = 1; segmentIndex <= safeSegmentCount; ++segmentIndex)
        {
            const Core::f32 t =
                static_cast<Core::f32>(segmentIndex) / static_cast<Core::f32>(safeSegmentCount);

            const Core::f32 angle = Core::TwoPi * t;

            const Core::f32 cosine = static_cast<Core::f32>(std::cos(angle));
            const Core::f32 sine = static_cast<Core::f32>(std::sin(angle));

            const Core::Vector3 currentPoint =
                center +
                axisA.Normalized() * (cosine * radius) +
                axisB.Normalized() * (sine * radius);

            mContext.RenderSystem->DrawDebugLine(previousPoint, currentPoint, color);

            previousPoint = currentPoint;
        }
    }

    void EditorToolModeController::DrawDebugBox(
        const Core::Vector3& center,
        const Core::f32 halfExtent,
        const Render::FRenderColor& color
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 x = Core::Vector3::Right() * halfExtent;
        const Core::Vector3 y = Core::Vector3::Up() * halfExtent;
        const Core::Vector3 z = Core::Vector3::Forward() * halfExtent;

        const Core::Vector3 p000 = center - x - y - z;
        const Core::Vector3 p001 = center - x - y + z;
        const Core::Vector3 p010 = center - x + y - z;
        const Core::Vector3 p011 = center - x + y + z;

        const Core::Vector3 p100 = center + x - y - z;
        const Core::Vector3 p101 = center + x - y + z;
        const Core::Vector3 p110 = center + x + y - z;
        const Core::Vector3 p111 = center + x + y + z;

        mContext.RenderSystem->DrawDebugLine(p000, p001, color);
        mContext.RenderSystem->DrawDebugLine(p001, p011, color);
        mContext.RenderSystem->DrawDebugLine(p011, p010, color);
        mContext.RenderSystem->DrawDebugLine(p010, p000, color);

        mContext.RenderSystem->DrawDebugLine(p100, p101, color);
        mContext.RenderSystem->DrawDebugLine(p101, p111, color);
        mContext.RenderSystem->DrawDebugLine(p111, p110, color);
        mContext.RenderSystem->DrawDebugLine(p110, p100, color);

        mContext.RenderSystem->DrawDebugLine(p000, p100, color);
        mContext.RenderSystem->DrawDebugLine(p001, p101, color);
        mContext.RenderSystem->DrawDebugLine(p010, p110, color);
        mContext.RenderSystem->DrawDebugLine(p011, p111, color);
    }

    Core::Vector3 EditorToolModeController::GetMarkerCenter() const
    {
        if (!mContext.MainCamera)
            return Core::Vector3::Zero();

        return mContext.MainCamera->GetPosition() +
            mContext.MainCamera->GetForwardVector() * mMarkerDistance;
    }

    Core::Vector3 EditorToolModeController::GetCameraRightVector() const
    {
        if (!mContext.MainCamera)
            return Core::Vector3::Right();

        const Core::Vector3 right = mContext.MainCamera->GetRightVector().Normalized();

        if (right.LengthSquared() <= 0.00001f)
            return Core::Vector3::Right();

        return right;
    }

    Core::Vector3 EditorToolModeController::GetCameraUpVector() const
    {
        if (!mContext.MainCamera)
            return Core::Vector3::Up();

        const Core::Vector3 up = mContext.MainCamera->GetUpVector().Normalized();

        if (up.LengthSquared() <= 0.00001f)
            return Core::Vector3::Up();

        return up;
    }
}