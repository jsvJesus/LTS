#include "EditorRuntime.h"

#include "EditorViewportController.h"
#include "EditorWorldController.h"
#include "EditorToolModeController.h"
#include "EditorSelectionController.h"
#include "EditorPickingController.h"

#include "World/EntityId.h"

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

        mLastSelectionPickRequestId = 0;

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

        mWorldController = std::make_unique<EditorWorldController>();

        FEditorWorldControllerDesc worldDesc {};
        worldDesc.CreateDefaultScene = true;
        worldDesc.EnableDebugDraw = true;
        worldDesc.DebugBoxHalfExtent = 0.35f;
        worldDesc.DebugAxisLength = 0.85f;
        worldDesc.PickRadius = 0.60f;

        if (!mWorldController->Initialize(mContext, worldDesc))
        {
            mWorldController.reset();

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

            mWorldController->Shutdown();
            mWorldController.reset();

            mViewportController->Shutdown();
            mViewportController.reset();

            mInitialized = false;
            return false;
        }

        mPickingController = std::make_unique<EditorPickingController>();

        FEditorPickingControllerDesc pickingDesc {};
        pickingDesc.EnableDebugRay = true;
        pickingDesc.LogPickRequests = true;
        pickingDesc.DebugRayLength = 12.0f;

        if (!mPickingController->Initialize(mContext, pickingDesc))
        {
            mPickingController.reset();

            mSelectionController->Shutdown();
            mSelectionController.reset();

            mWorldController->Shutdown();
            mWorldController.reset();

            mViewportController->Shutdown();
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

            mPickingController->Shutdown();
            mPickingController.reset();

            mSelectionController->Shutdown();
            mSelectionController.reset();

            mWorldController->Shutdown();
            mWorldController.reset();

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
        if (mToolModeController)
        {
            mToolModeController->Shutdown();
            mToolModeController.reset();
        }

        if (mPickingController)
        {
            mPickingController->Shutdown();
            mPickingController.reset();
        }

        if (mSelectionController)
        {
            mSelectionController->Shutdown();
            mSelectionController.reset();
        }

        if (mWorldController)
        {
            mWorldController->Shutdown();
            mWorldController.reset();
        }

        if (mViewportController)
        {
            mViewportController->Shutdown();
            mViewportController.reset();
        }

        mContext = Engine::FApplicationRuntimeContext {};

        mLastSelectionPickRequestId = 0;
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

        if (mWorldController)
        {
            mWorldController->Tick(deltaSeconds);
        }

        if (mPickingController)
        {
            mPickingController->Tick(deltaSeconds);
        }

        RoutePickingToSelection();

        if (mSelectionController)
        {
            mSelectionController->Tick(deltaSeconds);
        }

        SyncWorldSelectionDebug();

        if (mToolModeController)
        {
            mToolModeController->Tick(deltaSeconds);
        }

        // Позже тут будут:
        // gizmo tick
        // editor tools tick
        // inspector sync
    }

    void LevelEditorRuntime::RenderDebug()
    {
        if (!mInitialized)
            return;

        if (mViewportController)
        {
            mViewportController->RenderDebug();
        }

        if (mWorldController)
        {
            mWorldController->RenderDebug();
        }

        if (mPickingController)
        {
            mPickingController->RenderDebug();
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

    void LevelEditorRuntime::RoutePickingToSelection()
    {
        if (!mWorldController || !mPickingController || !mSelectionController)
            return;

        if (!mPickingController->HasLastPickRequest())
            return;

        const FEditorPickRequest& pickRequest =
            mPickingController->GetLastPickRequest();

        if (pickRequest.RequestId == mLastSelectionPickRequestId)
            return;

        mLastSelectionPickRequestId = pickRequest.RequestId;

        FEditorWorldPickResult pickResult {};

        if (mWorldController->TryPickEntity(pickRequest.Ray, pickResult))
        {
            mSelectionController->SetSelectedId(
                static_cast<EditorSelectionId>(pickResult.EntityId)
            );

            mWorldController->SetSelectedEntityId(pickResult.EntityId);
            return;
        }

        mSelectionController->ClearSelection();
        mWorldController->ClearSelectedEntityId();
    }

    void LevelEditorRuntime::SyncWorldSelectionDebug()
    {
        if (!mWorldController || !mSelectionController)
            return;

        if (!mSelectionController->HasSelection())
        {
            mWorldController->ClearSelectedEntityId();
            return;
        }

        mWorldController->SetSelectedEntityId(
            static_cast<World::EntityId>(mSelectionController->GetSelectedId())
        );
    }
}