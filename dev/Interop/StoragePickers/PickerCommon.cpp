// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "shellapi.h"
#include "PickerCommon.h"
#include <wil/resource.h>
#include "ShObjIdl.h"
#include "shobjidl_core.h"
#include <KnownFolders.h>
#include <filesystem>
#include <format>
#include <utility>


namespace {

    GUID HashHStringToGuid(winrt::hstring const& input)
    {
        auto algorithm = winrt::Windows::Security::Cryptography::Core::HashAlgorithmProvider::OpenAlgorithm(winrt::Windows::Security::Cryptography::Core::HashAlgorithmNames::Md5());

        auto buffer = winrt::Windows::Security::Cryptography::CryptographicBuffer::ConvertStringToBinary(input, winrt::Windows::Security::Cryptography::BinaryStringEncoding::Utf16LE);

        auto hash = algorithm.HashData(buffer);

        if (hash.Length() != sizeof(GUID))
        {
            throw winrt::hresult_error(E_FAIL, L"Invalid hash length");
        }

        winrt::com_array<uint8_t> resultBuffer(sizeof(GUID));
        winrt::Windows::Security::Cryptography::CryptographicBuffer::CopyToByteArray(hash, resultBuffer);
        GUID guid = *(reinterpret_cast<GUID*>(resultBuffer.data()));

        // Adjust the GUID to conform to version 3 UUID (MD5-based)
        guid.Data3 = (guid.Data3 & 0x0FFF) | 0x3000; // Set the version to 3
        guid.Data4[0] = (guid.Data4[0] & 0x3F) | 0x80; // Set variant to RFC 4122

        return guid;
    }

    winrt::com_ptr<IShellItem> GetKnownFolderFromId(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId pickerLocationId)
    {
        KNOWNFOLDERID knownFolderId;
        switch (pickerLocationId)
        {
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary:
            knownFolderId = FOLDERID_Documents;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::ComputerFolder:
            knownFolderId = FOLDERID_ComputerFolder;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::Desktop:
            knownFolderId = FOLDERID_Desktop;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::Downloads:
            knownFolderId = FOLDERID_Downloads;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::HomeGroup:
            knownFolderId = FOLDERID_HomeGroup;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::MusicLibrary:
            knownFolderId = FOLDERID_MusicLibrary;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::PicturesLibrary:
            knownFolderId = FOLDERID_PicturesLibrary;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::VideosLibrary:
            knownFolderId = FOLDERID_VideosLibrary;
            break;
        case winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::Objects3D:
            knownFolderId = FOLDERID_Objects3D;
            break;
        default:
            return nullptr;
        }

        auto knownFolderManager = winrt::create_instance<IKnownFolderManager>(CLSID_KnownFolderManager);

        winrt::com_ptr<IKnownFolder> knownFolder{};
        winrt::hresult hr = knownFolderManager->GetFolder(knownFolderId, knownFolder.put());
        if (!knownFolder)
        {
            knownFolderManager->GetFolder(FOLDERID_Documents, knownFolder.put());
        }
        if (!knownFolder)
        {
            return nullptr;
        }

        winrt::com_ptr<IShellItem> defaultFolder{};
        hr = knownFolder->GetShellItem(0, IID_PPV_ARGS(defaultFolder.put()));
        if (FAILED(hr))
        {
            return nullptr;
        }

        return defaultFolder;
    }

}

namespace PickerCommon {
    using namespace winrt;

    bool IsHStringNullOrEmpty(winrt::hstring value)
    {
        return value.empty();
    }

    winrt::hstring GetPathFromShellItem(winrt::com_ptr<IShellItem> shellItem)
    {
        wil::unique_cotaskmem_string filePath;
        check_hresult(shellItem->GetDisplayName(SIGDN_FILESYSPATH, filePath.put()));
        return winrt::hstring{ filePath.get() };
    }

    std::pair<winrt::com_ptr<IShellItem>, std::wstring> ParseFolderItemAndFileName(winrt::hstring const& filePath)
    {
        std::filesystem::path path(filePath.c_str());
        if (path.empty())
        {
            return { nullptr, L"" };
        }

        auto folderPath = path.parent_path();
        if (folderPath.empty())
        {
            // If the path does not have a parent, we cannot set folder.
            return { nullptr, L"" };
        }

        // If the parent folder does not exist or is not a directory, we cannot set folder.
        if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
        {
            return { nullptr, L"" };
        }

        winrt::com_ptr<IShellItem> shellItem;
        HRESULT hr = SHCreateItemFromParsingName(folderPath.c_str(), nullptr, IID_PPV_ARGS(shellItem.put()));
        if (SUCCEEDED(hr))
        {
            auto fileName = path.filename().wstring();
            return { shellItem, fileName };
        }

        // If the shellitem cannot be created, we cannot set the folder.
        return { nullptr, L""};
    }

    winrt::hstring PickerParameters::FormatExtensionWithWildcard(winrt::hstring extension)
    {
        if (!extension.empty() && extension[0] == L'*')
        {
            return extension;
        }
        else
        {
            return L"*" + extension;
        }
    }

    winrt::hstring PickerParameters::JoinExtensions(winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> extensions)
    {
        winrt::hstring result;
        bool first = true;
        for (const auto& ext : extensions)
        {
            if (first)
            {
                result = FormatExtensionWithWildcard(ext);
                first = false;
            }
            else
            {
                result = result + L";" + FormatExtensionWithWildcard(ext);
            }
        }
        return result;
    }

    /// <summary>
    /// Capture and processing pickers filter inputs and convert them into Common Item Dialog's accepting type, for FileOpenPicker
    /// </summary>
    /// <param name="filters">winrt style filters</param>
    void PickerParameters::CaptureFilterSpec(winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> filters)
    {
        size_t resultSize = filters.Size() + 1;   // A vector input will have unioned All Files category appended.
        FileTypeFilterData.clear();
        FileTypeFilterData.reserve(resultSize * static_cast<size_t>(2));

        std::wstring allFilesExtensionList;
        allFilesExtensionList.reserve(resultSize * 8); // a typical extension takes 6-7 chars, like "*.txt;" and "*.docx;"
        for (const auto& filter : filters)
        {
            auto ext = FormatExtensionWithWildcard(filter);
            FileTypeFilterData.push_back(L"");
            FileTypeFilterData.push_back(ext);

            allFilesExtensionList += ext;
            allFilesExtensionList += L";";
        }

        if (!allFilesExtensionList.empty())
        {
            allFilesExtensionList.pop_back(); // Remove the last semicolon
        }

        if (filters.Size() == 0)
        {
            // when filters not defined, set filter to All Files *.*
            FileTypeFilterData.push_back(AllFilesText);
            FileTypeFilterData.push_back(L"*");
        }
        else if (filters.Size() == 1 && allFilesExtensionList == L"*")
        {
            // when there're only one filter "*", set filter to All Files *.* (override the values pushed above)
            FileTypeFilterData[0] = AllFilesText;
            FileTypeFilterData[1] = L"*";
            resultSize = 1;
        }
        else
        {
            FileTypeFilterData.push_back(L"");
            FileTypeFilterData.push_back(allFilesExtensionList.c_str());
        }

        FileTypeFilterPara.clear();
        FileTypeFilterPara.reserve(resultSize);
        for (size_t i = 0; i < resultSize; i++)
        {
            FileTypeFilterPara.push_back({ FileTypeFilterData.at(i * 2).c_str(), FileTypeFilterData.at(i * 2 + 1).c_str() });
        }
    }

    /// <summary>
    /// Capture and processing pickers filter inputs and convert them into Common Item Dialog's accepting type, for FileSavePicker
    /// </summary>
    /// <param name="filters">winrt style filters</param>
    void PickerParameters::CaptureFilterSpec(winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::Collections::IVector<hstring>> filters)
    {
        size_t resultSize = filters.Size();
        FileTypeFilterData.clear();
        FileTypeFilterData.reserve(filters.Size() * static_cast<size_t>(2));

        for (const auto& filter : filters)
        {
            FileTypeFilterData.push_back(filter.Key());
            auto extensionList = JoinExtensions(filter.Value().GetView());
            FileTypeFilterData.push_back(extensionList);
        }

        if (filters.Size() == 0)
        {
            // when filters not defined, set filter to All Files *.*
            FileTypeFilterData.push_back(AllFilesText);
            FileTypeFilterData.push_back(L"*");
            resultSize = 1;
        }

        FileTypeFilterPara.clear();
        FileTypeFilterPara.reserve(resultSize);
        for (size_t i = 0; i < resultSize; i++)
        {
            FileTypeFilterPara.push_back({ FileTypeFilterData.at(i * 2).c_str(), FileTypeFilterData.at(i * 2 + 1).c_str() });
        }
    }

    void PickerParameters::ConfigureDialog(winrt::com_ptr<IFileDialog> dialog)
    {
        if (!IsHStringNullOrEmpty(CommitButtonText))
        {
            check_hresult(dialog->SetOkButtonLabel(CommitButtonText.c_str()));
        }

        if (!IsHStringNullOrEmpty(SettingsIdentifierId))
        {
            auto guid = HashHStringToGuid(SettingsIdentifierId);
            check_hresult(dialog->SetClientGuid(guid));
        }

        auto defaultFolder = GetKnownFolderFromId(PickerLocationId);
        if (defaultFolder != nullptr)
        {
            check_hresult(dialog->SetDefaultFolder(defaultFolder.get()));
        }

        check_hresult(dialog->SetFileTypes((UINT)FileTypeFilterPara.size(), FileTypeFilterPara.data()));
    }

    /// <summary>
    /// Configure the FileSaveDialog, this is only for FileSavePicker.
    /// </summary>
    /// <param name="dialog"></param>
    void PickerParameters::ConfigureFileSaveDialog(winrt::com_ptr<IFileSaveDialog> dialog)
    {
        winrt::hstring fileNameToSet;
        if (!IsHStringNullOrEmpty(SuggestedFileName))
        {
            fileNameToSet = SuggestedFileName;
        }
        
        if (!PickerCommon::IsHStringNullOrEmpty(SuggestedSaveFilePath))
        {
            auto result = ParseFolderItemAndFileName(SuggestedSaveFilePath);
            winrt::com_ptr<IShellItem> folderItem = result.first;
            fileNameToSet = result.second;
            if (folderItem)
            {
                check_hresult(dialog->SetFolder(folderItem.get()));
            }
        }

        // Set the filename (either from SuggestedSaveFilePath or SuggestedFileName)
        if (!PickerCommon::IsHStringNullOrEmpty(fileNameToSet))
        {
            check_hresult(dialog->SetFileName(fileNameToSet.c_str()));
        }

    }
}
