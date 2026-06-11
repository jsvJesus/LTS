#include "EditorSelectionController.h"

#include "Engine/Camera/Camera.h"

#include "Platform/Input.h"

#include "Render/RenderSystem.h"
#include "Render/RHI/RenderTypes.h"

#include "Core/Logger.h"

#include <sstream>

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
    }

    bool EditorSelectionController::Initialize(
        const Engine::FApplicationRuntimeContext& context,
        const FEditorSelectionControllerDesc& desc
    )
    {
        mContext = context;

        if (!mContext.RenderSystem)
        {
            mInitialized = false;
            return false;
        }

        mSelectionState = FEditorSelectionState {};

        mDebugMarkerEnabled = desc.EnableDebugMarker;

        mMarkerDistance = SanitizePositiveValue(desc.MarkerDistance, 4.25f);
        mMarkerSize = SanitizePositiveValue(desc.MarkerSize, 0.45f);

        mInitialized = true;

        Core::Logger::Info("Editor", "Editor selection controller initialized. SelectedId=none.");

        return true;
    }

    void EditorSelectionController::Shutdown()
    {
        if (mInitialized)
        {
            Core::Logger::Info("Editor", "Editor selection controller shutdown.");
        }

        mContext = Engine::FApplicationRuntimeContext {};
        mSelectionState = FEditorSelectionState {};

        mInitialized = false;
        mDebugMarkerEnabled = true;

        mMarkerDistance = 4.25f;
        mMarkerSize = 0.45f;
    }

    void EditorSelectionController::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized)
            return;

        HandleHotkeys();
    }

    void EditorSelectionController::RenderDebug()
    {
        if (!mInitialized || !mDebugMarkerEnabled || !mContext.RenderSystem)
            return;

        DrawSelectionStateMarker();
    }

    bool EditorSelectionController::SetSelectedId(const EditorSelectionId selectedId)
    {
        if (!IsValidEditorSelectionId(selectedId))
        {
            return ClearSelection();
        }

        if (mSelectionState.SelectedId == selectedId)
            return false;

        mSelectionState.SelectedId = selectedId;
        ++mSelectionState.Revision;

        if (mInitialized)
        {
            LogSelectionChanged();
        }

        return true;
    }

    bool EditorSelectionController::ClearSelection()
    {
        if (!mSelectionState.HasSelection())
            return false;

        mSelectionState.SelectedId = InvalidEditorSelectionId;
        ++mSelectionState.Revision;

        if (mInitialized)
        {
            LogSelectionCleared();
        }

        return true;
    }

    void EditorSelectionController::SetDebugMarkerEnabled(const bool enabled)
    {
        mDebugMarkerEnabled = enabled;
    }

    void EditorSelectionController::HandleHotkeys()
    {
        if (!mContext.InputSystem)
            return;

        // Пока нет реальных объектов, Escape только очищает состояние выбора.
        if (mContext.InputSystem->IsKeyPressed(Platform::KeyCode::Escape))
        {
            ClearSelection();
        }
    }

    void EditorSelectionController::LogSelectionChanged() const
    {
        std::ostringstream stream;
        stream << "Editor selection changed: SelectedId="
               << mSelectionState.SelectedId
               << ", Revision="
               << mSelectionState.Revision
               << ".";

        Core::Logger::Info("Editor", stream.str());
    }

    void EditorSelectionController::LogSelectionCleared() const
    {
        std::ostringstream stream;
        stream << "Editor selection cleared. Revision="
               << mSelectionState.Revision
               << ".";

        Core::Logger::Info("Editor", stream.str());
    }

    void EditorSelectionController::DrawSelectionStateMarker()
    {
        const Core::Vector3 center = GetMarkerCenter();

        if (mSelectionState.HasSelection())
        {
            DrawSelectedMarker(center, mMarkerSize);
        }
        else
        {
            DrawNoSelectionMarker(center, mMarkerSize);
        }
    }

    void EditorSelectionController::DrawNoSelectionMarker(
        const Core::Vector3& center,
        const Core::f32 size
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 right = GetCameraRightVector() * size;
        const Core::Vector3 up = GetCameraUpVector() * size;

        const Render::FRenderColor color =
            MakeColor(0.35f, 0.35f, 0.38f, 1.0f);

        mContext.RenderSystem->DrawDebugLine(
            center - right,
            center + right,
            color
        );

        mContext.RenderSystem->DrawDebugLine(
            center - up,
            center + up,
            color
        );
    }

    void EditorSelectionController::DrawSelectedMarker(
        const Core::Vector3& center,
        const Core::f32 size
    )
    {
        if (!mContext.RenderSystem)
            return;

        const Render::FRenderColor boxColor =
            MakeColor(1.00f, 0.55f, 0.10f, 1.0f);

        const Render::FRenderColor axisColor =
            MakeColor(1.00f, 0.80f, 0.25f, 1.0f);

        DrawDebugBox(center, size, boxColor);

        mContext.RenderSystem->DrawDebugLine(
            center,
            center + Core::Vector3::Up() * (size * 1.75f),
            axisColor
        );
    }

    void EditorSelectionController::DrawDebugBox(
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

    Core::Vector3 EditorSelectionController::GetMarkerCenter() const
    {
        if (!mContext.MainCamera)
            return Core::Vector3::Zero();

        return mContext.MainCamera->GetPosition() +
            mContext.MainCamera->GetForwardVector() * mMarkerDistance;
    }

    Core::Vector3 EditorSelectionController::GetCameraRightVector() const
    {
        if (!mContext.MainCamera)
            return Core::Vector3::Right();

        const Core::Vector3 right = mContext.MainCamera->GetRightVector().Normalized();

        if (right.LengthSquared() <= 0.00001f)
            return Core::Vector3::Right();

        return right;
    }

    Core::Vector3 EditorSelectionController::GetCameraUpVector() const
    {
        if (!mContext.MainCamera)
            return Core::Vector3::Up();

        const Core::Vector3 up = mContext.MainCamera->GetUpVector().Normalized();

        if (up.LengthSquared() <= 0.00001f)
            return Core::Vector3::Up();

        return up;
    }
}