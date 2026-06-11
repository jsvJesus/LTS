#include "World.h"

#include "Scene.h"
#include "Core/Logger.h"

namespace World
{
    World::~World()
    {
        Shutdown();
    }

    bool World::Initialize()
    {
        if (mInitialized)
            return true;

        mInitialized = true;

        Core::Logger::Info("World", "World initialized.");

        return true;
    }

    void World::Shutdown()
    {
        if (!mInitialized && !mActiveScene)
            return;

        mActiveScene.reset();
        mActiveSceneName.clear();

        if (mInitialized)
        {
            Core::Logger::Info("World", "World shutdown.");
        }

        mInitialized = false;
    }

    Scene* World::CreateEmptyScene(const Core::StringView sceneName)
    {
        if (!mInitialized)
            return nullptr;

        mActiveScene = std::make_unique<Scene>();
        mActiveSceneName = Core::String(sceneName.data(), sceneName.size());

        Core::Logger::Info("World", "Empty scene created.");

        return mActiveScene.get();
    }
}