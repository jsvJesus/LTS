#pragma once

#include "Engine/ApplicationRuntime.h"

namespace Editor
{
    class LevelEditorRuntime final : public Engine::IApplicationRuntime
    {
    public:
        LevelEditorRuntime() = default;
        ~LevelEditorRuntime() override = default;

        LevelEditorRuntime(const LevelEditorRuntime&) = delete;
        LevelEditorRuntime& operator=(const LevelEditorRuntime&) = delete;

        const char* GetRuntimeName() const override;

        bool Initialize(const Engine::FApplicationRuntimeContext& context) override;
        void Shutdown() override;

        void Tick(double deltaSeconds) override;
        void RenderDebug() override;

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }

    private:
        Engine::FApplicationRuntimeContext mContext {};
        bool mInitialized = false;
    };
}