using System;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Platform.Storage;
using EditorUI.ViewModels;

namespace EditorUI.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
    }

    private MainWindowViewModel? ViewModel => DataContext as MainWindowViewModel;

    private async void SelectHeightmapButton_Click(object? sender, RoutedEventArgs e)
    {
        if (ViewModel is null)
            return;

        var files = await StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions
        {
            Title = "Выбери .r16 heightmap",
            AllowMultiple = false,
            FileTypeFilter =
            [
                new FilePickerFileType("Raw 16-bit heightmap")
                {
                    Patterns = ["*.r16"]
                },
                new FilePickerFileType("All files")
                {
                    Patterns = ["*.*"]
                }
            ]
        });

        if (files.Count == 0)
            return;

        var path = files[0].TryGetLocalPath();

        if (string.IsNullOrWhiteSpace(path))
        {
            ViewModel.StatusMessage = "Не удалось получить локальный путь к файлу.";
            return;
        }

        ViewModel.ImportHeightmapPath = path;
    }
}