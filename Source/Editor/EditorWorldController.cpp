#include "EditorWorldController.h"

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

    bool EditorWorldController::Initialize(
        const Engine::FApplicationRuntimeContext& context,
        const FEditorWorldControllerDesc& desc
    )
    {
        mContext = context;

        if (!mContext.RenderSystem)
        {
            mInitialized = false;
            return false;
        }

        if (!mWorld.Initialize())
        {
            mInitialized = false;
            return false;
        }

        mDebugDrawEnabled = desc.EnableDebugDraw;
        mDebugBoxHalfExtent = SanitizePositiveValue(desc.DebugBoxHalfExtent, 0.35f);
        mDebugAxisLength = SanitizePositiveValue(desc.DebugAxisLength, 0.85f);

        if (desc.CreateDefaultScene && !CreateDefaultEditorScene())
        {
            mWorld.Shutdown();
            mInitialized = false;
            return false;
        }

        mInitialized = true;

        Core::Logger::Info("Editor", "Editor world controller initialized.");

        return true;
    }

    void EditorWorldController::Shutdown()
    {
        if (mInitialized)
        {
            Core::Logger::Info("Editor", "Editor world controller shutdown.");
        }

        mWorld.Shutdown();

        mContext = Engine::FApplicationRuntimeContext {};

        mInitialized = false;
        mDebugDrawEnabled = true;
        mDebugBoxHalfExtent = 0.35f;
        mDebugAxisLength = 0.85f;
    }

    void EditorWorldController::Tick(const double deltaSeconds)
    {
        (void)deltaSeconds;

        if (!mInitialized)
            return;
    }

    void EditorWorldController::RenderDebug()
    {
        if (!mInitialized || !mDebugDrawEnabled || !mContext.RenderSystem)
            return;

        DrawWorldDebug();
    }

    bool EditorWorldController::CreateDefaultEditorScene()
    {
        World::Scene* scene = mWorld.CreateEmptyScene("EditorPreviewScene");

        if (!scene)
            return false;

        World::FTransform originTransform {};
        originTransform.Position = Core::Vector3(0.0f, 0.5f, 0.0f);
        originTransform.Rotation = Core::Rotator::Zero();
        originTransform.Scale = Core::Vector3::One();

        scene->CreateEntity("OriginEntity", originTransform);

        World::FTransform rightTransform {};
        rightTransform.Position = Core::Vector3(2.0f, 0.5f, 0.0f);
        rightTransform.Rotation = Core::Rotator(0.0f, 35.0f, 0.0f);
        rightTransform.Scale = Core::Vector3::One();

        scene->CreateEntity("RightEntity", rightTransform);

        World::FTransform forwardTransform {};
        forwardTransform.Position = Core::Vector3(0.0f, 0.5f, 2.0f);
        forwardTransform.Rotation = Core::Rotator(0.0f, -35.0f, 0.0f);
        forwardTransform.Scale = Core::Vector3::One();

        scene->CreateEntity("ForwardEntity", forwardTransform);

        std::ostringstream stream;
        stream << "Default editor scene created. EntityCount="
               << scene->GetEntityCount()
               << ".";

        Core::Logger::Info("Editor", stream.str());

        return true;
    }

    void EditorWorldController::DrawWorldDebug()
    {
        const World::Scene* scene = mWorld.GetActiveScene();

        if (!scene)
            return;

        for (const World::FEntity& entity : scene->GetEntities())
        {
            if (!entity.IsValid())
                continue;

            DrawEntityDebug(entity);
        }
    }

    void EditorWorldController::DrawEntityDebug(const World::FEntity& entity)
    {
        if (!mContext.RenderSystem)
            return;

        const Core::Vector3 position = entity.Transform.Position;

        const Render::FRenderColor boxColor =
            MakeColor(0.95f, 0.72f, 0.18f, 1.0f);

        const Render::FRenderColor rightColor =
            MakeColor(1.00f, 0.20f, 0.20f, 1.0f);

        const Render::FRenderColor upColor =
            MakeColor(0.20f, 1.00f, 0.30f, 1.0f);

        const Render::FRenderColor forwardColor =
            MakeColor(0.20f, 0.45f, 1.00f, 1.0f);

        DrawDebugBox(position, mDebugBoxHalfExtent, boxColor);

        mContext.RenderSystem->DrawDebugLine(
            position,
            position + entity.Transform.GetRightVector() * mDebugAxisLength,
            rightColor
        );

        mContext.RenderSystem->DrawDebugLine(
            position,
            position + entity.Transform.GetUpVector() * mDebugAxisLength,
            upColor
        );

        mContext.RenderSystem->DrawDebugLine(
            position,
            position + entity.Transform.GetForwardVector() * mDebugAxisLength,
            forwardColor
        );
    }

    void EditorWorldController::DrawDebugBox(
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
}