using EditorUI.Models;

namespace EditorUI.ViewModels;

public sealed class EditorMapItemViewModel
{
    public EditorMapItemViewModel(EditorMapInfo map)
    {
        MapName = map.Document.Name;
        DisplayName = map.Document.DisplayName;
        MapPath = map.MapFilePath;

        TerrainInfo = map.Document.Terrain.Enabled
            ? $"{map.Document.Terrain.Width}x{map.Document.Terrain.Height}, Cell={map.Document.Terrain.CellSize}, MaxHeight={map.Document.Terrain.MaxHeight}"
            : "Terrain disabled";
    }

    public string MapName { get; }
    public string DisplayName { get; }
    public string MapPath { get; }
    public string TerrainInfo { get; }
}