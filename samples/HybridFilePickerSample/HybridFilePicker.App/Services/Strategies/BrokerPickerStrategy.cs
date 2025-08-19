using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Threading.Tasks;
using Windows.ApplicationModel.AppService;
using Windows.Foundation.Collections;

namespace HybridFilePicker.App.Services.Strategies
{
    public class BrokerPickerStrategy : IFilePickerService
    {
        private const string AppServiceName = "com.example.filepickerbroker";

        public static async Task<bool> IsBrokerAvailableAsync()
        {
            try
            {
                using var conn = new AppServiceConnection
                {
                    AppServiceName = AppServiceName,
                    PackageFamilyName = Windows.ApplicationModel.Package.Current.Id.FamilyName
                };

                var status = await conn.OpenAsync();
                if (status != AppServiceConnectionStatus.Success)
                    return false;

                var ping = new ValueSet { { "action", "ping" } };
                var resp = await conn.SendMessageAsync(ping);
                return resp.Status == AppServiceResponseStatus.Success && 
                       resp.Message.ContainsKey("pong");
            }
            catch
            {
                return false;
            }
        }

        public async Task<IReadOnlyList<PickedFile>> PickOpenFilesAsync(PickOptions options)
        {
            using var conn = new AppServiceConnection
            {
                AppServiceName = AppServiceName,
                PackageFamilyName = Windows.ApplicationModel.Package.Current.Id.FamilyName
            };

            var status = await conn.OpenAsync();
            if (status != AppServiceConnectionStatus.Success)
                throw new InvalidOperationException("Broker not available");

            var req = new ValueSet
            {
                { "action", "openFiles" },
                { "filters", string.Join(";", options.FileTypeFilters) },
                { "multi", options.AllowMultiple }
            };

            var resp = await conn.SendMessageAsync(req);
            if (resp.Status != AppServiceResponseStatus.Success)
                return Array.Empty<PickedFile>();

            if (!resp.Message.TryGetValue("payload", out var payloadObj))
                return Array.Empty<PickedFile>();

            var json = payloadObj?.ToString() ?? "[]";
            var brokerFiles = JsonSerializer.Deserialize<List<BrokerFileDto>>(json) ?? new();

            var list = new List<PickedFile>();
            foreach (var bf in brokerFiles)
            {
                byte[]? content = null;
                if (bf.Base64Content != null)
                {
                    content = Convert.FromBase64String(bf.Base64Content);
                }

                list.Add(new PickedFile(bf.Id, bf.DisplayName, bf.Extension, content, bf.TempPath));
            }

            return list;
        }

        private record BrokerFileDto(string Id, string DisplayName, string Extension, string? Base64Content, string? TempPath);
    }
}