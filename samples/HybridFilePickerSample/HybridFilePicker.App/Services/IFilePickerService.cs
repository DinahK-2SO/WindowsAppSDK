using System.Collections.Generic;
using System.Threading.Tasks;

namespace HybridFilePicker.App.Services
{
    public record PickOptions(IEnumerable<string> FileTypeFilters, bool AllowMultiple);

    public record PickedFile(string Id, string DisplayName, string SuggestedExtension, byte[]? InlineContent, string? TempPath);

    public interface IFilePickerService
    {
        Task<IReadOnlyList<PickedFile>> PickOpenFilesAsync(PickOptions options);
    }
}