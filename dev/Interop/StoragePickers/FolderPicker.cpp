// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "FolderPicker.h"
#include "Microsoft.Windows.Storage.Pickers.FolderPicker.g.cpp"
#include "StoragePickersTelemetry.h"
#include <shobjidl.h>
#include <shobjidl_core.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <windows.h>
#include "PickerCommon.h"
#include "PickFolderResult.h"

namespace winrt::Microsoft::Windows::Storage::Pickers::implementation
{
    FolderPicker::FolderPicker(winrt::Microsoft::UI::WindowId const& windowId)
        : m_windowId(windowId)
    {
        THROW_HR_IF(E_NOTIMPL, !::Microsoft::Windows::Storage::Pickers::Feature_StoragePickers::IsEnabled());
    }
    winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode FolderPicker::ViewMode()
    {
        return m_viewMode;
    }
    void FolderPicker::ViewMode(winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode const& value)
    {
        m_viewMode = value;
    }
    hstring FolderPicker::SettingsIdentifier()
    {
        return m_settingsIdentifier;
    }
    void FolderPicker::SettingsIdentifier(hstring const& value)
    {
        m_settingsIdentifier = value;
    }
    winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId FolderPicker::SuggestedStartLocation()
    {
        return m_suggestedStartLocation;
    }
    void FolderPicker::SuggestedStartLocation(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId const& value)
    {
        m_suggestedStartLocation = value;
    }
    hstring FolderPicker::CommitButtonText()
    {
        return m_commitButtonText;
    }
    void FolderPicker::CommitButtonText(hstring const& value)
    {
        m_commitButtonText = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> FolderPicker::FileTypeFilter()
    {
        return m_fileTypeFilter;
    }

    void FolderPicker::CaptureParameters(PickerCommon::PickerParameters& parameters)
    {
        parameters.HWnd = winrt::Microsoft::UI::GetWindowFromWindowId(m_windowId);
        parameters.CommitButtonText = m_commitButtonText;
        parameters.SettingsIdentifierId = m_settingsIdentifier;
        parameters.PickerLocationId = m_suggestedStartLocation;
        parameters.FileTypeFilterPara = PickerCommon::CaptureFilterSpec(parameters.FileTypeFilterData, m_fileTypeFilter.GetView());
    }


    winrt::Windows::Foundation::IAsyncOperation<winrt::Microsoft::Windows::Storage::Pickers::PickFolderResult> FolderPicker::PickSingleFolderAsync()
    {
        // TODO: remove get strong reference when telementry is safe stop
        auto lifetime{ get_strong() };

        auto logTelemetry{ StoragePickersTelemetry::FolderPickerPickSingleFolder::Start(m_telemetryHelper) };

        PickerCommon::PickerParameters parameters{};
        CaptureParameters(parameters);

        auto cancellationToken = co_await winrt::get_cancellation_token();
        cancellationToken.enable_propagation(true);
        co_await winrt::resume_background();

        if (cancellationToken())
        {
            logTelemetry.Stop(m_telemetryHelper, false);
            co_return nullptr;
        }

        auto dialog = create_instance<IFileOpenDialog>(CLSID_FileOpenDialog, CLSCTX_INPROC_SERVER);

        parameters.ConfigureDialog(dialog);
        dialog->SetOptions(FOS_PICKFOLDERS);

        bool keepTrying = true;
        winrt::com_ptr<IShellItem> shellItem{};
        winrt::hstring path;
        
        while (keepTrying)
        {
            auto hr = dialog->Show(parameters.HWnd);
            if (FAILED(hr) || cancellationToken())
            {
                logTelemetry.Stop(m_telemetryHelper, false);
                co_return nullptr;
            }

            hr = dialog->GetResult(shellItem.put());
            
            if (FAILED(hr))
            {
                if (hr == E_ACCESSDENIED)
                {
                    // Show message box and continue loop to let user pick again
                    MessageBoxW(
                        parameters.HWnd,
                        L"You can't open this location using this program. Please try a different location.",
                        L"Select Folder",
                        MB_OK | MB_ICONINFORMATION
                    );
                    // Continue the loop to show the dialog again
                }
                else
                {
                    // Handle other failures
                    logTelemetry.Stop(m_telemetryHelper, false);
                    co_return nullptr;
                }
            }
            else
            {
                // Check if the selected item is "This PC" or another special folder
                // Special folders often can't be accessed through FILESYSPATH or are virtual locations
                wil::unique_cotaskmem_string displayName;
                hr = shellItem->GetDisplayName(SIGDN_FILESYSPATH, displayName.put());
                
                if (FAILED(hr) || !displayName)
                {
                    // SIGDN_FILESYSPATH failed, which happens with special folders like "This PC"
                    MessageBoxW(
                        parameters.HWnd,
                        L"You can't open this location using this program. Please try a different location.",
                        L"Select Folder",
                        MB_OK | MB_ICONINFORMATION
                    );
                    // Continue the loop to show the dialog again
                }
                else
                {
                    // Additional verification to check if it's a special folder
                    SFGAOF attributes = 0;
                    hr = shellItem->GetAttributes(SFGAO_FILESYSTEM, &attributes);
                    
                    if (FAILED(hr) || !(attributes & SFGAO_FILESYSTEM))
                    {
                        // Location is not a filesystem folder
                        MessageBoxW(
                            parameters.HWnd,
                            L"You can't open this location using this program. Please try a different location.",
                            L"Select Folder",
                            MB_OK | MB_ICONINFORMATION
                        );
                        // Continue the loop to show the dialog again
                    }
                    else
                    {
                        // Success - we have a valid filesystem path
                        path = winrt::hstring(displayName.get());
                        keepTrying = false;
                    }
                }
            }
        }

        if (cancellationToken())
        {
            logTelemetry.Stop(m_telemetryHelper, false);
            co_return nullptr;
        }

        auto result = make<winrt::Microsoft::Windows::Storage::Pickers::implementation::PickFolderResult>(path);

        logTelemetry.Stop(m_telemetryHelper, true);
        co_return result;
    }
}
