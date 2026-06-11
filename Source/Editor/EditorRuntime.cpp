#include "EditorRuntime.h"

#include "Engine/Camera/Camera.h"
#include "Render/RenderSystem.h"

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
    }

    const char* LevelEditorRuntime::GetRuntimeName() const
    {
        return "LevelEditorRuntime";
    }

    bool LevelEditorRuntime::Initialize(const Engine::FApplicationRuntimeContext& context)
    {
        mContext = context;

        if (!mContext.RenderSystem)
        {
            mInitialized = false;
            return false;
        }

        mInitialized = true;
        return true;
    }

    void LevelEditorRuntime::Shutdown()
    {
        mContext = Engine::FApplicationRuntimeContext {};
        mInitialized = false;
    }

    void LevelEditorRuntime::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized)
            return;

        // Позже тут будут:
        // editor viewport tick
        // selection tick
        // gizmo tick
        // editor tools tick
    }

    void LevelEditorRuntime::RenderDebug()
    {
        if (!mInitialized || !mContext.RenderSystem)
            return;

        const Render::FRenderColor editorAccentColor =
            MakeColor(0.10f, 0.85f, 1.00f, 1.0f);

        const Render::FRenderColor editorSoftColor =
            MakeColor(0.08f, 0.45f, 0.55f, 1.0f);

        mContext.RenderSystem->DrawDebugLine(
            Core::Vector3(-2.0f, 0.04f, -2.0f),
            Core::Vector3( 2.0f, 0.04f,  2.0f),
            editorAccentColor
        );

        mContext.RenderSystem->DrawDebugLine(
            Core::Vector3(-2.0f, 0.04f,  2.0f),
            Core::Vector3( 2.0f, 0.04f, -2.0f),
            editorAccentColor
        );

        mContext.RenderSystem->DrawDebugLine(
            Core::Vector3(-1.0f, 0.05f, 0.0f),
            Core::Vector3( 1.0f, 0.05f, 0.0f),
            editorSoftColor
        );

        mContext.RenderSystem->DrawDebugLine(
            Core::Vector3(0.0f, 0.05f, -1.0f),
            Core::Vector3(0.0f, 0.05f,  1.0f),
            editorSoftColor
        );

        if (mContext.MainCamera)
        {
            const Core::Vector3 cameraPosition = mContext.MainCamera->GetPosition();
            const Core::Vector3 cameraForward = mContext.MainCamera->GetForwardVector();

            mContext.RenderSystem->DrawDebugLine(
                cameraPosition + cameraForward * 0.5f,
                cameraPosition + cameraForward * 3.0f,
                editorAccentColor
            );
        }
    }
}