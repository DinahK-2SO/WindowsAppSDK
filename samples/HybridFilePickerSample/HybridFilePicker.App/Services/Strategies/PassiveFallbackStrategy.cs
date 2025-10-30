using System.Collections.Generic;
using System.Threading.Tasks;

namespace HybridFilePicker.App.Services.Strategies
{
    public class PassiveFallbackStrategy : IFilePickerService
    {
        public Task<IReadOnlyList<PickedFile>> PickOpenFilesAsync(PickOptions options)
        {
            // Return empty results - UI will show guidance for drag & drop
            return Task.FromResult<IReadOnlyList<PickedFile>>(new List<PickedFile>());
        }
    }
}