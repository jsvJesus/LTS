#pragma once

#include <memory>

#include "Engine/ApplicationRuntime.h"

#include "EditorViewportController.h"

namespace Editor
{
    class LevelEditorRuntime final : public Engine::IApplicationRuntime
    {
    public:
        LevelEditorRuntime();
        ~LevelEditorRuntime() override;

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

        std::unique_ptr<EditorViewportController> mViewportController;

        bool mInitialized = false;
    };
}