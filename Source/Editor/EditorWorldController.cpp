#include "EditorWorldController.h"

#include "Render/RenderSystem.h"
#include "Render/RHI/RenderTypes.h"

#include "Core/Logger.h"

#include <limits>
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

        bool IntersectRaySphere(
            const Core::Vector3& rayOrigin,
            const Core::Vector3& rayDirection,
            const Core::Vector3& sphereCenter,
            const Core::f32 sphereRadius,
            Core::f32& outDistance
        )
        {
            const Core::Vector3 direction = rayDirection.Normalized();

            if (direction.LengthSquared() <= 0.00001f)
                return false;

            const Core::Vector3 oc = rayOrigin - sphereCenter;

            const Core::f32 a = Core::Vector3::Dot(direction, direction);
            const Core::f32 b = 2.0f * Core::Vector3::Dot(oc, direction);
            const Core::f32 c = Core::Vector3::Dot(oc, oc) - sphereRadius * sphereRadius;

            const Core::f32 discriminant = b * b - 4.0f * a * c;

            if (discriminant < 0.0f)
                return false;

            const Core::f32 sqrtDiscriminant = Core::SafeSqrt(discriminant);
            const Core::f32 invDenominator = 1.0f / (2.0f * a);

            const Core::f32 t0 = (-b - sqrtDiscriminant) * invDenominator;
            const Core::f32 t1 = (-b + sqrtDiscriminant) * invDenominator;

            if (t0 >= 0.0f)
            {
                outDistance = t0;
                return true;
            }

            if (t1 >= 0.0f)
            {
                outDistance = t1;
                return true;
            }

            return false;
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

        mSelectedEntityId = World::InvalidEntityId;

        mDebugDrawEnabled = desc.EnableDebugDraw;
        mDebugBoxHalfExtent = SanitizePositiveValue(desc.DebugBoxHalfExtent, 0.35f);
        mDebugAxisLength = SanitizePositiveValue(desc.DebugAxisLength, 0.85f);
        mPickRadius = SanitizePositiveValue(desc.PickRadius, 0.60f);

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

        mSelectedEntityId = World::InvalidEntityId;

        mInitialized = false;
        mDebugDrawEnabled = true;
        mDebugBoxHalfExtent = 0.35f;
        mDebugAxisLength = 0.85f;
        mPickRadius = 0.60f;
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

    bool EditorWorldController::TryPickEntity(
        const FEditorPickRay& ray,
        FEditorWorldPickResult& outResult
    ) const
    {
        outResult = FEditorWorldPickResult {};

        if (!mInitialized || !ray.IsValid())
            return false;

        const World::Scene* scene = mWorld.GetActiveScene();

        if (!scene)
            return false;

        const Core::Vector3 rayDirection = ray.Direction.Normalized();

        if (rayDirection.LengthSquared() <= 0.00001f)
            return false;

        Core::f32 bestDistance = std::numeric_limits<Core::f32>::max();
        World::EntityId bestEntityId = World::InvalidEntityId;

        for (const World::FEntity& entity : scene->GetEntities())
        {
            if (!entity.IsValid())
                continue;

            Core::f32 distance = 0.0f;

            if (!IntersectRaySphere(
                ray.Origin,
                rayDirection,
                entity.Transform.Position,
                mPickRadius,
                distance
            ))
            {
                continue;
            }

            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestEntityId = entity.Id;
            }
        }

        if (!World::IsValidEntityId(bestEntityId))
            return false;

        outResult.Hit = true;
        outResult.EntityId = bestEntityId;
        outResult.Distance = bestDistance;
        outResult.Position = ray.Origin + rayDirection * bestDistance;

        return true;
    }

    void EditorWorldController::SetSelectedEntityId(const World::EntityId entityId)
    {
        mSelectedEntityId = World::IsValidEntityId(entityId)
            ? entityId
            : World::InvalidEntityId;
    }

    void EditorWorldController::ClearSelectedEntityId()
    {
        mSelectedEntityId = World::InvalidEntityId;
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

        const bool selected = entity.Id == mSelectedEntityId;

        const Core::Vector3 position = entity.Transform.Position;

        const Render::FRenderColor boxColor = selected
            ? MakeColor(1.00f, 0.28f, 0.05f, 1.0f)
            : MakeColor(0.95f, 0.72f, 0.18f, 1.0f);

        const Render::FRenderColor rightColor =
            MakeColor(1.00f, 0.20f, 0.20f, 1.0f);

        const Render::FRenderColor upColor =
            MakeColor(0.20f, 1.00f, 0.30f, 1.0f);

        const Render::FRenderColor forwardColor =
            MakeColor(0.20f, 0.45f, 1.00f, 1.0f);

        const Core::f32 boxHalfExtent = selected
            ? mDebugBoxHalfExtent * 1.35f
            : mDebugBoxHalfExtent;

        DrawDebugBox(position, boxHalfExtent, boxColor);

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