using System.Text;

namespace EditorUI.Services;

public static class MapNameSanitizer
{
    public static string Sanitize(string value)
    {
        if (string.IsNullOrWhiteSpace(value))
            return string.Empty;

        var builder = new StringBuilder(value.Length);

        foreach (var character in value.Trim())
        {
            if (char.IsLetterOrDigit(character))
            {
                builder.Append(char.ToLowerInvariant(character));
                continue;
            }

            if (character is '_' or '-' or ' ')
            {
                builder.Append('_');
            }
        }

        var result = builder.ToString();

        while (result.Contains("__", StringComparison.Ordinal))
        {
            result = result.Replace("__", "_", StringComparison.Ordinal);
        }

        return result.Trim('_');
    }
}