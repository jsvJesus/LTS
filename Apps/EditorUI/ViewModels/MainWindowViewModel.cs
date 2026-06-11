using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using EditorUI.Models;
using EditorUI.Services;

namespace EditorUI.ViewModels;

public partial class MainWindowViewModel : ViewModelBase
{
    private readonly MapProjectService _mapService;

    [ObservableProperty]
    private string _newMapName = "test_map";

    [ObservableProperty]
    private string _terrainWidthText = "1025";

    [ObservableProperty]
    private string _terrainHeightText = "1025";

    [ObservableProperty]
    private string _cellSizeText = "1.0";

    [ObservableProperty]
    private string _maxHeightText = "1000.0";

    [ObservableProperty]
    private string _importHeightmapPath = string.Empty;

    [ObservableProperty]
    private string _statusMessage = "Готово.";

    [ObservableProperty]
    private EditorMapItemViewModel? _selectedMap;

    public string ProjectRoot { get; }

    public ObservableCollection<EditorMapItemViewModel> Maps { get; } = [];

    public MainWindowViewModel()
    {
        ProjectRoot = ProjectPaths.FindProjectRoot();
        _mapService = new MapProjectService(ProjectRoot);

        RefreshMaps();
    }

    [RelayCommand]
    private void RefreshMaps()
    {
        try
        {
            Maps.Clear();

            foreach (var map in _mapService.FindMaps())
            {
                Maps.Add(new EditorMapItemViewModel(map));
            }

            StatusMessage = Maps.Count == 0
                ? "Карты не найдены. Создай новую карту или импортируй .r16."
                : $"Найдено карт: {Maps.Count}.";
        }
        catch (Exception exception)
        {
            StatusMessage = $"Ошибка обновления списка карт: {exception.Message}";
        }
    }

    [RelayCommand]
    private async Task CreateMapAsync()
    {
        try
        {
            var request = BuildMapCreateRequest(null);

            var result = await _mapService.CreateEmptyMapAsync(request);

            StatusMessage =
                $"Карта создана: {result.Document.DisplayName}\n{result.MapFilePath}";

            RefreshMaps();

            SelectedMap = FindMapByPath(result.MapFilePath);
        }
        catch (Exception exception)
        {
            StatusMessage = $"Ошибка создания карты: {exception.Message}";
        }
    }

    [RelayCommand]
    private async Task ImportMapAsync()
    {
        try
        {
            if (string.IsNullOrWhiteSpace(ImportHeightmapPath))
            {
                StatusMessage = "Сначала выбери .r16 файл.";
                return;
            }

            if (!File.Exists(ImportHeightmapPath))
            {
                StatusMessage = $"Файл не найден: {ImportHeightmapPath}";
                return;
            }

            var request = BuildMapCreateRequest(ImportHeightmapPath);

            var result = await _mapService.ImportHeightmapMapAsync(request);

            StatusMessage =
                $"Карта импортирована: {result.Document.DisplayName}\n{result.MapFilePath}";

            RefreshMaps();

            SelectedMap = FindMapByPath(result.MapFilePath);
        }
        catch (Exception exception)
        {
            StatusMessage = $"Ошибка импорта карты: {exception.Message}";
        }
    }

    [RelayCommand]
    private void OpenSelectedMap()
    {
        try
        {
            if (SelectedMap is null)
            {
                StatusMessage = "Выбери карту из списка.";
                return;
            }

            var levelEditorPath = ProjectPaths.FindLevelEditorExecutable(ProjectRoot);

            if (string.IsNullOrWhiteSpace(levelEditorPath) || !File.Exists(levelEditorPath))
            {
                StatusMessage =
                    "LevelEditor.exe не найден.\n" +
                    "Сначала собери LevelEditor в Release|x64.\n" +
                    $"Ожидал: {Path.Combine(ProjectRoot, "Bin", "x64", "Release", "LevelEditor.exe")}";
                return;
            }

            var startInfo = new ProcessStartInfo
            {
                FileName = levelEditorPath,
                WorkingDirectory = ProjectRoot,
                UseShellExecute = false
            };

            startInfo.ArgumentList.Add($"--map={SelectedMap.MapPath}");

            Process.Start(startInfo);

            StatusMessage =
                $"Запускаю LevelEditor:\n{levelEditorPath}\n--map={SelectedMap.MapPath}";
        }
        catch (Exception exception)
        {
            StatusMessage = $"Ошибка запуска LevelEditor: {exception.Message}";
        }
    }

    [RelayCommand]
    private void OpenMapsFolder()
    {
        try
        {
            var mapsDirectory = _mapService.MapsDirectory;

            Directory.CreateDirectory(mapsDirectory);

            Process.Start(new ProcessStartInfo
            {
                FileName = mapsDirectory,
                UseShellExecute = true
            });

            StatusMessage = $"Открыта папка: {mapsDirectory}";
        }
        catch (Exception exception)
        {
            StatusMessage = $"Ошибка открытия папки карт: {exception.Message}";
        }
    }

    private MapCreateRequest BuildMapCreateRequest(string? sourceHeightmapPath)
    {
        var mapName = MapNameSanitizer.Sanitize(NewMapName);

        if (string.IsNullOrWhiteSpace(mapName))
            throw new InvalidOperationException("Имя карты пустое или некорректное.");

        var width = ParseInt(TerrainWidthText, "Width");
        var height = ParseInt(TerrainHeightText, "Height");
        var cellSize = ParseFloat(CellSizeText, "Cell size");
        var maxHeight = ParseFloat(MaxHeightText, "Max height");

        if (width <= 1 || height <= 1)
            throw new InvalidOperationException("Width/Height должны быть больше 1.");

        if (cellSize <= 0.0f)
            throw new InvalidOperationException("Cell size должен быть больше 0.");

        if (maxHeight <= 0.0f)
            throw new InvalidOperationException("Max height должен быть больше 0.");

        return new MapCreateRequest(
            mapName,
            mapName,
            width,
            height,
            cellSize,
            maxHeight,
            sourceHeightmapPath
        );
    }

    private EditorMapItemViewModel? FindMapByPath(string mapPath)
    {
        foreach (var map in Maps)
        {
            if (string.Equals(map.MapPath, mapPath, StringComparison.OrdinalIgnoreCase))
                return map;
        }

        return null;
    }

    private static int ParseInt(string value, string fieldName)
    {
        if (!int.TryParse(
            value,
            NumberStyles.Integer,
            CultureInfo.InvariantCulture,
            out var result
        ))
        {
            throw new InvalidOperationException($"{fieldName}: некорректное число.");
        }

        return result;
    }

    private static float ParseFloat(string value, string fieldName)
    {
        if (!float.TryParse(
            value,
            NumberStyles.Float,
            CultureInfo.InvariantCulture,
            out var result
        ))
        {
            throw new InvalidOperationException($"{fieldName}: некорректное число. Используй точку, например 1.0.");
        }

        return result;
    }
}