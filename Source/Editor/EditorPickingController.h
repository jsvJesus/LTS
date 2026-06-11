#pragma once

#include "EditorPickingTypes.h"

#include "Engine/ApplicationRuntime.h"

namespace Editor
{
    struct FEditorPickingControllerDesc final
    {
        bool EnableDebugRay = true;
        bool LogPickRequests = true;

        Core::f32 DebugRayLength = 12.0f;
    };

    class EditorPickingController final
    {
    public:
        EditorPickingController() = default;
        ~EditorPickingController() = default;

        EditorPickingController(const EditorPickingController&) = delete;
        EditorPickingController& operator=(const EditorPickingController&) = delete;

        bool Initialize(
            const Engine::FApplicationRuntimeContext& context,
            const FEditorPickingControllerDesc& desc
        );

        void Shutdown();

        void Tick(double deltaSeconds);
        void RenderDebug();

        void SetDebugRayEnabled(bool enabled);
        void SetPickRequestLoggingEnabled(bool enabled);

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }
        [[nodiscard]] bool IsDebugRayEnabled() const { return mDebugRayEnabled; }
        [[nodiscard]] bool IsPickRequestLoggingEnabled() const { return mLogPickRequests; }

        [[nodiscard]] bool HasLastPickRequest() const { return mLastPickRequest.IsValid(); }
        [[nodiscard]] const FEditorPickRequest& GetLastPickRequest() const { return mLastPickRequest; }

        bool BuildCurrentPickRay(FEditorPickRay& outRay) const;

    private:
        void HandleInput();

        bool BuildPickRequest(FEditorPickRequest& outRequest);
        bool BuildPickRay(FEditorPickRay& outRay) const;

        void LogPickRequest(const FEditorPickRequest& request) const;
        void DrawLastPickRay();

        [[nodiscard]] Core::f32 GetSafeDebugRayLength() const;

    private:
        Engine::FApplicationRuntimeContext mContext {};

        FEditorPickRequest mLastPickRequest {};

        bool mInitialized = false;
        bool mDebugRayEnabled = true;
        bool mLogPickRequests = true;

        Core::f32 mDebugRayLength = 12.0f;
        Core::u64 mNextRequestId = 1;
    };
}