#pragma once

#include "EditorPickingTypes.h"

#include "Engine/ApplicationRuntime.h"

#include "World/World.h"

namespace Render
{
    struct FRenderColor;
}

namespace Editor
{
    struct FEditorWorldPickResult final
    {
        bool Hit = false;

        World::EntityId EntityId = World::InvalidEntityId;

        Core::f32 Distance = 0.0f;
        Core::Vector3 Position = Core::Vector3::Zero();

        [[nodiscard]] bool IsValid() const
        {
            return Hit && World::IsValidEntityId(EntityId);
        }
    };

    struct FEditorWorldControllerDesc final
    {
        bool CreateDefaultScene = true;
        bool EnableDebugDraw = true;

        Core::f32 DebugBoxHalfExtent = 0.35f;
        Core::f32 DebugAxisLength = 0.85f;

        // Пока это простой radius вокруг debug entity.
        // Позже заменим на bounds/physics/scene query.
        Core::f32 PickRadius = 0.60f;
    };

    class EditorWorldController final
    {
    public:
        EditorWorldController() = default;
        ~EditorWorldController() = default;

        EditorWorldController(const EditorWorldController&) = delete;
        EditorWorldController& operator=(const EditorWorldController&) = delete;

        bool Initialize(
            const Engine::FApplicationRuntimeContext& context,
            const FEditorWorldControllerDesc& desc
        );

        void Shutdown();

        void Tick(double deltaSeconds);
        void RenderDebug();

        bool TryPickEntity(
            const FEditorPickRay& ray,
            FEditorWorldPickResult& outResult
        ) const;

        void SetSelectedEntityId(World::EntityId entityId);
        void ClearSelectedEntityId();

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }

        [[nodiscard]] World::World* GetWorld() { return &mWorld; }
        [[nodiscard]] const World::World* GetWorld() const { return &mWorld; }

        bool GetEntityTransform(
            World::EntityId entityId,
            World::FTransform& outTransform
        ) const;

        bool SetEntityTransform(
            World::EntityId entityId,
            const World::FTransform& transform
        );

        [[nodiscard]] World::EntityId GetSelectedEntityId() const { return mSelectedEntityId; }

    private:
        bool CreateDefaultEditorScene();

        void DrawWorldDebug();
        void DrawEntityDebug(const World::FEntity& entity);

        void DrawDebugBox(
            const Core::Vector3& center,
            Core::f32 halfExtent,
            const Render::FRenderColor& color
        );

    private:
        Engine::FApplicationRuntimeContext mContext {};

        World::World mWorld;

        World::EntityId mSelectedEntityId = World::InvalidEntityId;

        bool mInitialized = false;
        bool mDebugDrawEnabled = true;

        Core::f32 mDebugBoxHalfExtent = 0.35f;
        Core::f32 mDebugAxisLength = 0.85f;
        Core::f32 mPickRadius = 0.60f;
    };
}