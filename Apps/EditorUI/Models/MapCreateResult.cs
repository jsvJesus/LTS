namespace EditorUI.Models;

public sealed record MapCreateResult(
    string MapDirectory,
    string MapFilePath,
    MapDocument Document
);