#pragma once

#include "Engine.h"

namespace Platform
{
    class InputSystem;
}

namespace Render
{
    class RenderSystem;
}

namespace Engine
{
    class Camera;
    class CameraController;

    struct FApplicationRuntimeContext final
    {
        EApplicationMode ApplicationMode = EApplicationMode::Unknown;

        Platform::InputSystem* InputSystem = nullptr;
        Render::RenderSystem* RenderSystem = nullptr;

        Camera* MainCamera = nullptr;
        CameraController* MainCameraController = nullptr;
    };

    class IApplicationRuntime
    {
    public:
        virtual ~IApplicationRuntime() = default;

        virtual const char* GetRuntimeName() const = 0;

        virtual bool Initialize(const FApplicationRuntimeContext& context)
        {
            (void)context;
            return true;
        }

        virtual void Shutdown()
        {
        }

        virtual void Tick(double deltaSeconds)
        {
            (void)deltaSeconds;
        }

        virtual void RenderDebug()
        {
        }
    };
}