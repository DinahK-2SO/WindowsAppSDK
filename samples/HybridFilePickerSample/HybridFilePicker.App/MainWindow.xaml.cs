using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using HybridFilePicker.App.Services;
using HybridFilePicker.App.Services.Strategies;
using System.Linq;
using System.Threading.Tasks;

namespace HybridFilePicker.App
{
    public sealed partial class MainWindow : Window
    {
        private IFilePickerService? _picker;

        public MainWindow()
        {
            this.InitializeComponent();
            _ = InitAsync();
        }

        private async Task InitAsync()
        {
            _picker = await FilePickerStrategyResolver.ResolveAsync();
        }

        private async void OnPickClicked(object sender, RoutedEventArgs e)
        {
            if (_picker == null) return;

            var files = await _picker.PickOpenFilesAsync(new PickOptions(new[] { ".txt", ".md" }, true));
            
            Output.Text = files.Count == 0 
                ? "No files (maybe need broker or drag & drop)" 
                : string.Join("\n", files.Select(f => $"{f.DisplayName} (inline={(f.InlineContent != null)}, temp={f.TempPath})"));
        }
    }
}