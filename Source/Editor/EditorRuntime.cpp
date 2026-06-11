#include "EditorRuntime.h"

#include "EditorViewportController.h"
#include "EditorWorldController.h"
#include "EditorToolModeController.h"
#include "EditorSelectionController.h"
#include "EditorPickingController.h"
#include "EditorGizmoController.h"

#include "World/EntityId.h"

#include "Platform/Input.h"

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

        mGizmoController = std::make_unique<EditorGizmoController>();

        FEditorGizmoControllerDesc gizmoDesc {};
        gizmoDesc.EnableDebugDraw = true;
        gizmoDesc.MoveAxisLength = 1.35f;
        gizmoDesc.RotateRadius = 0.85f;
        gizmoDesc.ScaleBoxHalfExtent = 0.42f;
        gizmoDesc.AxisHitRadius = 0.18f;

        if (!mGizmoController->Initialize(mContext, gizmoDesc))
        {
            mGizmoController.reset();

            mToolModeController->Shutdown();
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
        if (mGizmoController)
        {
            mGizmoController->Shutdown();
            mGizmoController.reset();
        }
        
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
        mLastGizmoPickRequestId = 0;
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

        const bool gizmoCapturedPick = RoutePickingToGizmo();

        const bool gizmoIsDragging =
            mGizmoController && mGizmoController->IsDragging();

        if (!gizmoCapturedPick && !gizmoIsDragging)
        {
            RoutePickingToSelection();
        }

        if (mSelectionController)
        {
            mSelectionController->Tick(deltaSeconds);
        }

        SyncWorldSelectionDebug();

        if (mToolModeController)
        {
            mToolModeController->Tick(deltaSeconds);
        }

        SyncGizmoState();
        UpdateGizmoDrag();

        if (mGizmoController)
        {
            mGizmoController->Tick(deltaSeconds);
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

        if (mGizmoController)
        {
            mGizmoController->RenderDebug();
        }
    }

    bool LevelEditorRuntime::RoutePickingToGizmo()
    {
        if (!mGizmoController || !mPickingController)
            return false;

        if (!mPickingController->HasLastPickRequest())
            return false;

        const FEditorPickRequest& pickRequest =
            mPickingController->GetLastPickRequest();

        if (pickRequest.RequestId == mLastGizmoPickRequestId)
            return true;

        if (pickRequest.RequestId == mLastSelectionPickRequestId)
            return false;

        FEditorGizmoAxisHitResult hitResult {};

        if (!mGizmoController->TryHitAxis(pickRequest.Ray, hitResult))
            return false;

        mLastGizmoPickRequestId = pickRequest.RequestId;

        mGizmoController->BeginDrag(pickRequest.Ray, hitResult.Axis);

        return true;
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

        if (pickRequest.RequestId == mLastGizmoPickRequestId)
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

    void LevelEditorRuntime::UpdateGizmoDrag()
    {
        if (!mGizmoController || !mPickingController || !mContext.InputSystem)
            return;

        if (!mGizmoController->IsDragging())
            return;

        if (!mContext.InputSystem->IsMouseButtonDown(Platform::MouseButton::Left))
            return;

        FEditorPickRay currentRay {};

        if (!mPickingController->BuildCurrentPickRay(currentRay))
            return;

        mGizmoController->UpdateDrag(currentRay);

        ApplyGizmoPreviewTransform();
    }

    void LevelEditorRuntime::ApplyGizmoPreviewTransform()
    {
        if (!mGizmoController || !mWorldController)
            return;

        if (!mGizmoController->IsDragging())
            return;

        const World::EntityId targetEntityId = mGizmoController->GetTargetEntityId();

        if (!World::IsValidEntityId(targetEntityId))
            return;

        mWorldController->SetEntityTransform(
            targetEntityId,
            mGizmoController->GetTargetTransform()
        );
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

    void LevelEditorRuntime::SyncGizmoState()
    {
        if (!mGizmoController || !mToolModeController || !mSelectionController || !mWorldController)
            return;

        mGizmoController->SetToolMode(mToolModeController->GetToolMode());

        if (!mSelectionController->HasSelection())
        {
            mGizmoController->ClearTarget();
            return;
        }

        const World::EntityId selectedEntityId =
            static_cast<World::EntityId>(mSelectionController->GetSelectedId());

        World::FTransform selectedTransform {};

        if (!mWorldController->GetEntityTransform(selectedEntityId, selectedTransform))
        {
            mGizmoController->ClearTarget();
            return;
        }

        mGizmoController->SetTarget(selectedEntityId, selectedTransform);
    }
}