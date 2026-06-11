using System.Text.Json;
using EditorUI.Models;

namespace EditorUI.Services;

public sealed class MapProjectService
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase
    };

    public MapProjectService(string projectRoot)
    {
        ProjectRoot = projectRoot;
        MapsDirectory = Path.Combine(ProjectRoot, "Content", "Maps");
    }

    public string ProjectRoot { get; }
    public string MapsDirectory { get; }

    public IEnumerable<EditorMapInfo> FindMaps()
    {
        if (!Directory.Exists(MapsDirectory))
            yield break;

        foreach (var mapFilePath in Directory.EnumerateFiles(
            MapsDirectory,
            "map.json",
            SearchOption.AllDirectories
        ))
        {
            MapDocument? document = null;

            try
            {
                var json = File.ReadAllText(mapFilePath);
                document = JsonSerializer.Deserialize<MapDocument>(json, JsonOptions);
            }
            catch
            {
                // Битую карту пока просто пропускаем.
            }

            if (document is null)
                continue;

            yield return new EditorMapInfo(
                Path.GetDirectoryName(mapFilePath) ?? MapsDirectory,
                mapFilePath,
                document
            );
        }
    }

    public async Task<MapCreateResult> CreateEmptyMapAsync(MapCreateRequest request)
    {
        var mapDirectory = BuildMapDirectory(request.Name);

        EnsureMapDirectoryIsAvailable(mapDirectory);

        Directory.CreateDirectory(mapDirectory);

        var terrainFilePath = Path.Combine(mapDirectory, "terrain.r16");

        await CreateEmptyHeightmapAsync(
            terrainFilePath,
            request.TerrainWidth,
            request.TerrainHeight
        );

        return await SaveMapDocumentAsync(mapDirectory, request);
    }

    public async Task<MapCreateResult> ImportHeightmapMapAsync(MapCreateRequest request)
    {
        if (string.IsNullOrWhiteSpace(request.SourceHeightmapPath))
            throw new InvalidOperationException("SourceHeightmapPath is empty.");

        if (!File.Exists(request.SourceHeightmapPath))
            throw new FileNotFoundException("Heightmap file not found.", request.SourceHeightmapPath);

        var mapDirectory = BuildMapDirectory(request.Name);

        EnsureMapDirectoryIsAvailable(mapDirectory);

        Directory.CreateDirectory(mapDirectory);

        var terrainFilePath = Path.Combine(mapDirectory, "terrain.r16");

        File.Copy(request.SourceHeightmapPath, terrainFilePath, overwrite: false);

        ValidateR16Size(
            terrainFilePath,
            request.TerrainWidth,
            request.TerrainHeight
        );

        return await SaveMapDocumentAsync(mapDirectory, request);
    }

    private async Task<MapCreateResult> SaveMapDocumentAsync(
        string mapDirectory,
        MapCreateRequest request
    )
    {
        var utcNow = DateTime.UtcNow.ToString("O");

        var document = new MapDocument
        {
            Version = 1,
            Name = request.Name,
            DisplayName = request.DisplayName,
            CreatedAtUtc = utcNow,
            UpdatedAtUtc = utcNow,
            Terrain = new TerrainDocument
            {
                Enabled = true,
                Heightmap = "terrain.r16",
                Format = "r16",
                Width = request.TerrainWidth,
                Height = request.TerrainHeight,
                CellSize = request.CellSize,
                MaxHeight = request.MaxHeight,
                LittleEndian = true
            },
            Entities = []
        };

        var mapFilePath = Path.Combine(mapDirectory, "map.json");

        var json = JsonSerializer.Serialize(document, JsonOptions);

        await File.WriteAllTextAsync(mapFilePath, json);

        return new MapCreateResult(mapDirectory, mapFilePath, document);
    }

    private string BuildMapDirectory(string mapName)
    {
        return Path.Combine(MapsDirectory, mapName);
    }

    private static void EnsureMapDirectoryIsAvailable(string mapDirectory)
    {
        if (Directory.Exists(mapDirectory))
            throw new InvalidOperationException($"Карта уже существует: {mapDirectory}");
    }

    private static async Task CreateEmptyHeightmapAsync(
        string filePath,
        int width,
        int height
    )
    {
        var pixelCount = checked(width * height);
        var byteCount = checked(pixelCount * 2);

        var buffer = new byte[1024 * 1024];

        await using var stream = new FileStream(
            filePath,
            FileMode.CreateNew,
            FileAccess.Write,
            FileShare.None
        );

        var remaining = byteCount;

        while (remaining > 0)
        {
            var writeSize = Math.Min(buffer.Length, remaining);
            await stream.WriteAsync(buffer.AsMemory(0, writeSize));
            remaining -= writeSize;
        }
    }

    private static void ValidateR16Size(
        string filePath,
        int width,
        int height
    )
    {
        var expectedBytes = checked((long)width * height * 2L);
        var actualBytes = new FileInfo(filePath).Length;

        if (actualBytes != expectedBytes)
        {
            throw new InvalidOperationException(
                $".r16 размер не совпадает. Ожидалось {expectedBytes} bytes, получено {actualBytes} bytes. " +
                $"Проверь Width/Height."
            );
        }
    }
}