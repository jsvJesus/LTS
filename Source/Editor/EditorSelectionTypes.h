#pragma once

#include "Core/BaseTypes.h"

namespace Editor
{
    using EditorSelectionId = Core::u64;

    constexpr EditorSelectionId InvalidEditorSelectionId = 0;

    [[nodiscard]] constexpr bool IsValidEditorSelectionId(const EditorSelectionId id)
    {
        return id != InvalidEditorSelectionId;
    }

    struct FEditorSelectionState final
    {
        EditorSelectionId SelectedId = InvalidEditorSelectionId;

        // Увеличивается при каждом реальном изменении выбора.
        // Позже UI/Inspector/Gizmo смогут быстро понимать, что selection state изменился.
        Core::u64 Revision = 0;

        [[nodiscard]] bool HasSelection() const
        {
            return IsValidEditorSelectionId(SelectedId);
        }
    };
}