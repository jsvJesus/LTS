#pragma once

#include "Engine/ApplicationRuntime.h"

#include "World/World.h"

namespace Render
{
    struct FRenderColor;
}

namespace Editor
{
    struct FEditorWorldControllerDesc final
    {
        bool CreateDefaultScene = true;
        bool EnableDebugDraw = true;

        Core::f32 DebugBoxHalfExtent = 0.35f;
        Core::f32 DebugAxisLength = 0.85f;
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

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }

        [[nodiscard]] World::World* GetWorld() { return &mWorld; }
        [[nodiscard]] const World::World* GetWorld() const { return &mWorld; }

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

        bool mInitialized = false;
        bool mDebugDrawEnabled = true;

        Core::f32 mDebugBoxHalfExtent = 0.35f;
        Core::f32 mDebugAxisLength = 0.85f;
    };
}