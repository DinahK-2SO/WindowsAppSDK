using System;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading.Tasks;
using Windows.ApplicationModel.AppService;
using Windows.ApplicationModel.Background;
using Windows.Foundation.Collections;

namespace HybridFilePicker.Broker
{
    public sealed class BrokerService : IBackgroundTask
    {
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            var deferral = taskInstance.GetDeferral();
            taskInstance.AppServiceConnection.RequestReceived += OnRequest;
            taskInstance.Canceled += (_, __) => deferral.Complete();
        }

        private async void OnRequest(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var deferral = args.GetDeferral();
            try
            {
                var msg = args.Request.Message;
                var action = msg["action"] as string;

                if (action == "ping")
                {
                    await args.Request.SendResponseAsync(new ValueSet { { "pong", "1" } });
                    return;
                }

                if (action == "openFiles")
                {
                    var filters = (msg["filters"] as string ?? "").Split(';', StringSplitOptions.RemoveEmptyEntries);
                    bool multi = msg.TryGetValue("multi", out var mObj) && mObj is bool b && b;

                    // Placeholder: real picker call omitted in sample; simulate
                    var list = filters.DefaultIfEmpty(".txt").Select(f => new BrokerFileDto
                    {
                        Id = Guid.NewGuid().ToString("N"),
                        DisplayName = $"Sample{f}",
                        Extension = f,
                        Base64Content = Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes("Hello")),
                        TempPath = null
                    }).ToList();

                    var json = JsonSerializer.Serialize(list);
                    await args.Request.SendResponseAsync(new ValueSet { { "payload", json } });
                }
            }
            finally
            {
                deferral.Complete();
            }
        }

        private class BrokerFileDto
        {
            public string Id { get; set; } = "";
            public string DisplayName { get; set; } = "";
            public string Extension { get; set; } = "";
            public string? Base64Content { get; set; }
            public string? TempPath { get; set; }
        }
    }
}