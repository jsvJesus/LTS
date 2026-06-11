#include "EditorRuntime.h"

#include "EditorViewportController.h"
#include "EditorToolModeController.h"
#include "EditorSelectionController.h"

namespace Editor
{
    LevelEditorRuntime::LevelEditorRuntime() = default;
    LevelEditorRuntime::~LevelEditorRuntime() = default;

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

        mViewportController = std::make_unique<EditorViewportController>();

        FEditorViewportControllerDesc viewportDesc {};
        viewportDesc.EnableViewportCamera = true;
        viewportDesc.EnableDebugOverlay = true;
        viewportDesc.DrawFocusMarker = true;
        viewportDesc.DrawCameraForwardLine = true;
        viewportDesc.FocusDistance = 5.0f;
        viewportDesc.FocusMarkerSize = 0.35f;

        if (!mViewportController->Initialize(mContext, viewportDesc))
        {
            mViewportController.reset();
            mInitialized = false;
            return false;
        }

        mToolModeController = std::make_unique<EditorToolModeController>();

        FEditorToolModeControllerDesc toolModeDesc {};
        toolModeDesc.InitialToolMode = EEditorToolMode::Select;
        toolModeDesc.EnableDebugMarker = true;
        toolModeDesc.MarkerDistance = 5.0f;
        toolModeDesc.MarkerSize = 0.75f;

        if (!mToolModeController->Initialize(mContext, toolModeDesc))
        {
            mToolModeController.reset();

            mViewportController->Shutdown();
            mViewportController.reset();

            mInitialized = false;
            return false;
        }

        mSelectionController = std::make_unique<EditorSelectionController>();

        FEditorSelectionControllerDesc selectionDesc {};
        selectionDesc.EnableDebugMarker = true;
        selectionDesc.MarkerDistance = 4.25f;
        selectionDesc.MarkerSize = 0.45f;

        if (!mSelectionController->Initialize(mContext, selectionDesc))
        {
            mSelectionController.reset();

            mToolModeController->Shutdown();
            mToolModeController.reset();

            mViewportController->Shutdown();
            mViewportController.reset();

            mInitialized = false;
            return false;
        }

        mInitialized = true;
        return true;
    }

    void LevelEditorRuntime::Shutdown()
    {
        if (mSelectionController)
        {
            mSelectionController->Shutdown();
            mSelectionController.reset();
        }

        if (mToolModeController)
        {
            mToolModeController->Shutdown();
            mToolModeController.reset();
        }

        if (mViewportController)
        {
            mViewportController->Shutdown();
            mViewportController.reset();
        }

        mContext = Engine::FApplicationRuntimeContext {};
        mInitialized = false;
    }

    void LevelEditorRuntime::Tick(const double deltaSeconds)
    {
        if (!mInitialized)
            return;

        if (mViewportController)
        {
            mViewportController->Tick(deltaSeconds);
        }

        if (mSelectionController)
        {
            mSelectionController->Tick(deltaSeconds);
        }

        if (mToolModeController)
        {
            mToolModeController->Tick(deltaSeconds);
        }

        // Позже тут будут:
        // selection picking
        // gizmo tick
        // editor tools tick
    }

    void LevelEditorRuntime::RenderDebug()
    {
        if (!mInitialized)
            return;

        if (mViewportController)
        {
            mViewportController->RenderDebug();
        }

        if (mSelectionController)
        {
            mSelectionController->RenderDebug();
        }

        if (mToolModeController)
        {
            mToolModeController->RenderDebug();
        }
    }
}