#pragma once

#include "EntityId.h"
#include "Transform.h"

#include "Core/BaseTypes.h"

namespace World
{
    struct FEntity final
    {
        EntityId Id = InvalidEntityId;

        Core::String Name;

        FTransform Transform {};

        bool Alive = false;

        [[nodiscard]] bool IsValid() const
        {
            return Alive && IsValidEntityId(Id);
        }
    };
}