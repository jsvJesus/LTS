#pragma once

#include <memory>

#include "Engine/ApplicationRuntime.h"

#include "EditorViewportController.h"
#include "EditorWorldController.h"
#include "EditorToolModeController.h"
#include "EditorSelectionController.h"
#include "EditorPickingController.h"
#include "EditorGizmoController.h"

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
        void RoutePickingToSelection();
        void SyncWorldSelectionDebug();
        void SyncGizmoState();

    private:
        Engine::FApplicationRuntimeContext mContext {};

        std::unique_ptr<EditorViewportController> mViewportController;
        std::unique_ptr<EditorWorldController> mWorldController;
        std::unique_ptr<EditorToolModeController> mToolModeController;
        std::unique_ptr<EditorSelectionController> mSelectionController;
        std::unique_ptr<EditorPickingController> mPickingController;
        std::unique_ptr<EditorGizmoController> mGizmoController;

        Core::u64 mLastSelectionPickRequestId = 0;

        bool mInitialized = false;
    };
}