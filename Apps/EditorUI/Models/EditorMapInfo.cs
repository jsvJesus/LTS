namespace EditorUI.Models;

public sealed record EditorMapInfo(
    string MapDirectory,
    string MapFilePath,
    MapDocument Document
);