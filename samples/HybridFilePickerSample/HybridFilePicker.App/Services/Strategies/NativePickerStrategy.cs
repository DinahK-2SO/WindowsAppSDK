using System.Collections.Generic;
using System.Threading.Tasks;

namespace HybridFilePicker.App.Services.Strategies
{
    public class NativePickerStrategy : IFilePickerService
    {
        public async Task<IReadOnlyList<PickedFile>> PickOpenFilesAsync(PickOptions options)
        {
            await Task.CompletedTask;
            
            // Placeholder: Real implementation would use Microsoft.Windows.Storage.Pickers when available in full trust
            return new List<PickedFile>();
        }
    }
}