#pragma once

#include "Core/BaseTypes.h"

namespace World
{
    using EntityId = Core::u64;

    constexpr EntityId InvalidEntityId = 0;

    [[nodiscard]] constexpr bool IsValidEntityId(const EntityId id)
    {
        return id != InvalidEntityId;
    }
}