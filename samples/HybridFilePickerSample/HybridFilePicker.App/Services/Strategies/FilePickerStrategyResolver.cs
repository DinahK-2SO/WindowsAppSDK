using System;
using System.Threading.Tasks;

namespace HybridFilePicker.App.Services.Strategies
{
    public static class FilePickerStrategyResolver
    {
        public static async Task<IFilePickerService> ResolveAsync()
        {
            // Check for full trust indicator (placeholder logic)
            if (Environment.GetEnvironmentVariable("FULLTRUST_INDICATOR") == "1")
            {
                return new NativePickerStrategy();
            }

            // Check if broker is available
            if (await BrokerPickerStrategy.IsBrokerAvailableAsync())
            {
                return new BrokerPickerStrategy();
            }

            // Fallback to passive strategy
            return new PassiveFallbackStrategy();
        }
    }
}