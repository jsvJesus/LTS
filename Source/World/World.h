#pragma once

#include "Scene.h"

#include "Core/BaseTypes.h"

namespace World
{
    class World final
    {
    public:
        World() = default;
        ~World();

        World(const World&) = delete;
        World& operator=(const World&) = delete;

        bool Initialize();
        void Shutdown();

        Scene* CreateEmptyScene(Core::StringView sceneName);

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }

        [[nodiscard]] Scene* GetActiveScene() { return mActiveScene.get(); }
        [[nodiscard]] const Scene* GetActiveScene() const { return mActiveScene.get(); }

        [[nodiscard]] Core::StringView GetActiveSceneName() const { return mActiveSceneName; }

    private:
        Core::UniquePtr<Scene> mActiveScene;
        Core::String mActiveSceneName;

        bool mInitialized = false;
    };
}