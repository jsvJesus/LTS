namespace EditorUI.Models;

public sealed class MapDocument
{
    public int Version { get; set; } = 1;

    public string Name { get; set; } = string.Empty;
    public string DisplayName { get; set; } = string.Empty;

    public string CreatedAtUtc { get; set; } = string.Empty;
    public string UpdatedAtUtc { get; set; } = string.Empty;

    public TerrainDocument Terrain { get; set; } = new();

    public List<EntityDocument> Entities { get; set; } = [];
}

public sealed class TerrainDocument
{
    public bool Enabled { get; set; } = true;

    public string Heightmap { get; set; } = "terrain.r16";
    public string Format { get; set; } = "r16";

    public int Width { get; set; } = 1025;
    public int Height { get; set; } = 1025;

    public float CellSize { get; set; } = 1.0f;
    public float MaxHeight { get; set; } = 1000.0f;

    public bool LittleEndian { get; set; } = true;
}

public sealed class EntityDocument
{
    public string Id { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;

    public TransformDocument Transform { get; set; } = new();
}

public sealed class TransformDocument
{
    public float PositionX { get; set; }
    public float PositionY { get; set; }
    public float PositionZ { get; set; }

    public float RotationPitch { get; set; }
    public float RotationYaw { get; set; }
    public float RotationRoll { get; set; }

    public float ScaleX { get; set; } = 1.0f;
    public float ScaleY { get; set; } = 1.0f;
    public float ScaleZ { get; set; } = 1.0f;
}