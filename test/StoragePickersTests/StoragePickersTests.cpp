// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"

#include <FrameworkUdk/Containment.h>
#include <winrt/Microsoft.Windows.Storage.Pickers.h>

namespace TB = ::Test::Bootstrap;
namespace TP = ::Test::Packages;

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace Test::StoragePickersTests
{
    class StoragePickersTests
    {
    public:
        BEGIN_TEST_CLASS(StoragePickersTests)
            TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA") // MTA is required for ::Test::Bootstrap::SetupPackages()
            TEST_CLASS_PROPERTY(L"RunFixtureAs:Class", L"RestrictedUser")
            //TEST_CLASS_PROPERTY(L"RunFixtureAs:Class", L"UAP")
            //TEST_CLASS_PROPERTY(L"RunAs", L"UAP")

            END_TEST_CLASS()

            TEST_CLASS_SETUP(ClassSetup)
        {
            ::Test::Bootstrap::SetupPackages();
            return true;
        }

        TEST_CLASS_CLEANUP(ClassCleanup)
        {
            ::Test::Bootstrap::CleanupPackages();
            return true;
        }

        TEST_METHOD_SETUP(MethodInit)
        {
            VERIFY_IS_TRUE(TP::IsPackageRegistered_WindowsAppRuntimeFramework());

            // The test method setup and execution is on a different thread than the class setup.
            // Initialize the framework for the test thread.
            ::Test::Bootstrap::SetupBootstrap();
            return true;
        }

        TEST_METHOD_CLEANUP(MethodUninit)
        {
            VERIFY_IS_TRUE(TP::IsPackageRegistered_WindowsAppRuntimeFramework());
            ::Test::Bootstrap::CleanupBootstrap();
            return true;
        }
       // The unit tests will be updated,first test might is there for testing purpose locally.
       // Focusing solely on functional tests for now.

       // Commenting out this test as it is an E2E scenario test that requires UI automation for pipeline execution.
       /*

        TEST_METHOD(FileOpenPicker_ShouldPickFile)
        {
            try
            {
                auto parentWindow = ::GetForegroundWindow();
                winrt::Microsoft::UI::WindowId windowId{ reinterpret_cast<uint64_t>(parentWindow) };
                winrt::Microsoft::Windows::Storage::Pickers::FileOpenPicker picker{ windowId };
                picker.FileTypeFilter().Append(L"*");
                // Act
                auto operation = picker.PickSingleFileAsync();
                auto file = operation.get();
                auto path = file.Path();
                // Assert
                if (file != nullptr)
                {
                    Log::Comment(L"File open was successful");
                }
                else
                {
                    Log::Error(L"File open canceled.");
                }
            }
            catch (const winrt::hresult_error& ex)
            {
                Log::Error((std::wstring(L"Exception thrown: ") + ex.message().c_str()).c_str());
                VERIFY_FAIL(L"Exception occurred during file open picker.");
            }
            catch (const std::exception& ex)
            {
                Log::Error((std::wstring(L"Standard exception thrown: ") + winrt::to_hstring(ex.what()).c_str()).c_str());
                VERIFY_FAIL(L"Standard exception occurred during file open picker.");
            }
        }

        TEST_METHOD(FileSavePicker_ShouldCreateNewFile)
        {
            try
            {
                auto parentWindow = ::GetForegroundWindow();
                winrt::Microsoft::UI::WindowId windowId{ reinterpret_cast<uint64_t>(parentWindow) };
                winrt::Microsoft::Windows::Storage::Pickers::FileSavePicker savePicker(windowId);
                //savePicker.SuggestedStartLocation(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);
                savePicker.FileTypeChoices().Insert(L"Plain Text", winrt::single_threaded_vector<winrt::hstring>({ L".txt" }));
                savePicker.SuggestedFileName(L"test.txt");
                // Act
                auto fileOperation = savePicker.PickSaveFileAsync();
                auto file = fileOperation.get();
                auto path = file.Path();

                // Assert
                if (file != nullptr)
                {
                    Log::Comment(L"File save was successful.");
                }
                else
                {
                    Log::Error(L"File save failed or was canceled.");
                }
            }
            catch (const winrt::hresult_error& ex)
            {
                Log::Error((std::wstring(L"Exception thrown: ") + ex.message().c_str()).c_str());
                VERIFY_FAIL(L"Exception occurred during file save picker.");
            }
            catch (const std::exception& ex)
            {
                Log::Error((std::wstring(L"Standard exception thrown: ") + winrt::to_hstring(ex.what()).c_str()).c_str());
                VERIFY_FAIL(L"Standard exception occurred during file save picker.");
            }
        }

       */


        TEST_METHOD(VerifyFileOpenPickerOptionsAreReadCorrectly)
        {
            winrt::Microsoft::UI::WindowId windowId{};
            winrt::Microsoft::Windows::Storage::Pickers::FileOpenPicker picker(windowId);

            picker.ViewMode(winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::List);
            VERIFY_ARE_EQUAL(picker.ViewMode(), winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::List);

            picker.ViewMode(winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::Thumbnail);
            VERIFY_ARE_EQUAL(picker.ViewMode(), winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::Thumbnail);

            picker.SettingsIdentifier(L"id");
            VERIFY_ARE_EQUAL(picker.SettingsIdentifier(), L"id");

            picker.SuggestedStartLocation(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);
            VERIFY_ARE_EQUAL(picker.SuggestedStartLocation(), winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);

            picker.CommitButtonText(L"commit");
            VERIFY_ARE_EQUAL(picker.CommitButtonText(), L"commit");

            picker.FileTypeFilter().Append(L"*");
            VERIFY_ARE_EQUAL(picker.FileTypeFilter().GetAt(0), L"*");
        }

        TEST_METHOD(VerifyFileSavePickerOptionsAreReadCorrectly)
        {
            winrt::Microsoft::UI::WindowId windowId{};
            winrt::Microsoft::Windows::Storage::Pickers::FileSavePicker picker(windowId);

            picker.SettingsIdentifier(L"id");
            VERIFY_ARE_EQUAL(picker.SettingsIdentifier(), L"id");

            picker.SuggestedStartLocation(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);
            VERIFY_ARE_EQUAL(picker.SuggestedStartLocation(), winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);

            picker.CommitButtonText(L"commit");
            VERIFY_ARE_EQUAL(picker.CommitButtonText(), L"commit");

            auto filters = winrt::single_threaded_vector<winrt::hstring>();
            filters.Append(L"*");
            picker.FileTypeChoices().Insert(L"All Files", filters);
            VERIFY_ARE_EQUAL(picker.FileTypeChoices().Lookup(L"All Files").GetAt(0), L"*");
        }

        TEST_METHOD(VerifyFileSavePickerSuggestedSaveFileInterface)
        {
            winrt::Microsoft::UI::WindowId windowId{};
            winrt::Microsoft::Windows::Storage::Pickers::FileSavePicker picker(windowId);

            // Test creating SuggestedSaveFile with a path
            auto suggestedSaveFile = winrt::Microsoft::Windows::Storage::Pickers::SuggestedSaveFile(L"C:\\temp\\testfile.txt");
            VERIFY_ARE_EQUAL(suggestedSaveFile.Path(), L"C:\\temp\\testfile.txt");

            // Test setting and getting SuggestedSaveFile on FileSavePicker
            picker.SuggestedSaveFile(suggestedSaveFile);
            auto retrievedSuggestedSaveFile = picker.SuggestedSaveFile();
            VERIFY_IS_NOT_NULL(retrievedSuggestedSaveFile);
            VERIFY_ARE_EQUAL(retrievedSuggestedSaveFile.Path(), L"C:\\temp\\testfile.txt");

            // Test setting to null
            picker.SuggestedSaveFile(nullptr);
            VERIFY_IS_NULL(picker.SuggestedSaveFile());
        }

        TEST_METHOD(VerifyFileSavePickerSuggestedSaveFileEdgeCases)
        {
            winrt::Microsoft::UI::WindowId windowId{};
            winrt::Microsoft::Windows::Storage::Pickers::FileSavePicker picker(windowId);

            // Test with empty path
            auto emptySuggestedSaveFile = winrt::Microsoft::Windows::Storage::Pickers::SuggestedSaveFile(L"");
            VERIFY_ARE_EQUAL(emptySuggestedSaveFile.Path(), L"");
            picker.SuggestedSaveFile(emptySuggestedSaveFile);
            VERIFY_IS_NOT_NULL(picker.SuggestedSaveFile());

            // Test with filename only (no directory)
            auto filenameOnlyFile = winrt::Microsoft::Windows::Storage::Pickers::SuggestedSaveFile(L"myfile.txt");
            VERIFY_ARE_EQUAL(filenameOnlyFile.Path(), L"myfile.txt");
            picker.SuggestedSaveFile(filenameOnlyFile);
            VERIFY_ARE_EQUAL(picker.SuggestedSaveFile().Path(), L"myfile.txt");

            // Test with directory only (no filename)
            auto directoryOnlyFile = winrt::Microsoft::Windows::Storage::Pickers::SuggestedSaveFile(L"C:\\temp\\");
            VERIFY_ARE_EQUAL(directoryOnlyFile.Path(), L"C:\\temp\\");
            picker.SuggestedSaveFile(directoryOnlyFile);
            VERIFY_ARE_EQUAL(picker.SuggestedSaveFile().Path(), L"C:\\temp\\");

            // Test with relative path
            auto relativePathFile = winrt::Microsoft::Windows::Storage::Pickers::SuggestedSaveFile(L"..\\Documents\\file.txt");
            VERIFY_ARE_EQUAL(relativePathFile.Path(), L"..\\Documents\\file.txt");
            picker.SuggestedSaveFile(relativePathFile);
            VERIFY_ARE_EQUAL(picker.SuggestedSaveFile().Path(), L"..\\Documents\\file.txt");
        }

        TEST_METHOD(VerifyFileSavePickerSuggestedSaveFileAndSuggestedFileNameCoexistence)
        {
            winrt::Microsoft::UI::WindowId windowId{};
            winrt::Microsoft::Windows::Storage::Pickers::FileSavePicker picker(windowId);

            // Test that both SuggestedSaveFile and SuggestedFileName can be set independently
            picker.SuggestedFileName(L"suggested.txt");
            VERIFY_ARE_EQUAL(picker.SuggestedFileName(), L"suggested.txt");

            auto suggestedSaveFile = winrt::Microsoft::Windows::Storage::Pickers::SuggestedSaveFile(L"C:\\temp\\priority.txt");
            picker.SuggestedSaveFile(suggestedSaveFile);
            
            // Verify both properties are set independently
            VERIFY_ARE_EQUAL(picker.SuggestedFileName(), L"suggested.txt");
            VERIFY_ARE_EQUAL(picker.SuggestedSaveFile().Path(), L"C:\\temp\\priority.txt");

            // Test clearing SuggestedSaveFile while keeping SuggestedFileName
            picker.SuggestedSaveFile(nullptr);
            VERIFY_IS_NULL(picker.SuggestedSaveFile());
            VERIFY_ARE_EQUAL(picker.SuggestedFileName(), L"suggested.txt");
        }

        TEST_METHOD(VerifyFolderPickerOptionsAreReadCorrectly)
        {
            auto parentWindow = ::GetForegroundWindow();
            winrt::Microsoft::UI::WindowId windowId{};
            winrt::Microsoft::Windows::Storage::Pickers::FolderPicker picker(windowId);

            picker.ViewMode(winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::List);
            VERIFY_ARE_EQUAL(picker.ViewMode(), winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::List);

            picker.ViewMode(winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::Thumbnail);
            VERIFY_ARE_EQUAL(picker.ViewMode(), winrt::Microsoft::Windows::Storage::Pickers::PickerViewMode::Thumbnail);

            picker.SettingsIdentifier(L"id");
            VERIFY_ARE_EQUAL(picker.SettingsIdentifier(), L"id");

            picker.SuggestedStartLocation(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);
            VERIFY_ARE_EQUAL(picker.SuggestedStartLocation(), winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);

            picker.CommitButtonText(L"commit");
            VERIFY_ARE_EQUAL(picker.CommitButtonText(), L"commit");

            picker.FileTypeFilter().Append(L"*");
            VERIFY_ARE_EQUAL(picker.FileTypeFilter().GetAt(0), L"*");
        }
    };
}
