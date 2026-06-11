namespace EditorUI.Services;

public static class ProjectPaths
{
    public static string FindProjectRoot()
    {
        var directory = new DirectoryInfo(AppContext.BaseDirectory);

        while (directory is not null)
        {
            if (File.Exists(Path.Combine(directory.FullName, "LTS.sln")) ||
                Directory.Exists(Path.Combine(directory.FullName, "Apps")) &&
                Directory.Exists(Path.Combine(directory.FullName, "Source")))
            {
                return directory.FullName;
            }

            directory = directory.Parent;
        }

        return Directory.GetCurrentDirectory();
    }

    public static string FindLevelEditorExecutable(string projectRoot)
    {
        var releasePath = Path.Combine(projectRoot, "Bin", "x64", "Release", "LevelEditor.exe");

        if (File.Exists(releasePath))
            return releasePath;

        var debugPath = Path.Combine(projectRoot, "Bin", "x64", "Debug", "LevelEditor.exe");

        if (File.Exists(debugPath))
            return debugPath;

        return releasePath;
    }
}