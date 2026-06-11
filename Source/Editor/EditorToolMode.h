#pragma once

#include "Core/BaseTypes.h"

namespace Editor
{
    enum class EEditorToolMode : Core::u8
    {
        Select = 0,
        Move,
        Rotate,
        Scale
    };

    [[nodiscard]] constexpr Core::StringView GetEditorToolModeName(const EEditorToolMode mode)
    {
        switch (mode)
        {
        case EEditorToolMode::Select:
            return "Select";

        case EEditorToolMode::Move:
            return "Move";

        case EEditorToolMode::Rotate:
            return "Rotate";

        case EEditorToolMode::Scale:
            return "Scale";

        default:
            return "Unknown";
        }
    }
}