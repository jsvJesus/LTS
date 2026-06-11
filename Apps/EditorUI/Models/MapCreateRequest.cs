namespace EditorUI.Models;

public sealed record MapCreateRequest(
    string Name,
    string DisplayName,
    int TerrainWidth,
    int TerrainHeight,
    float CellSize,
    float MaxHeight,
    string? SourceHeightmapPath
);