// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License. See LICENSE in the project root for license information.

// Forward-declare the various AutoInitialize functions
namespace Microsoft::Windows::ApplicationModel::DynamicDependency::Bootstrap
{
    namespace AutoInitialize { void Initialize(); void Shutdown(); };
}
namespace Microsoft::Windows::ApplicationModel::WindowsAppRuntime::DeploymentManager
{
    namespace AutoInitialize { void Initialize(); };
}
namespace Microsoft::Windows::Foundation::UndockedRegFreeWinRT
{
    namespace AutoInitialize { void Initialize(); };
}
namespace Microsoft::Windows::ApplicationModel::WindowsAppRuntime::Compatibility
{
    namespace AutoInitialize { void Initialize(); };
}

// The common AutoInitialize struct that calls the various AutoInitialize functions
namespace Microsoft::Windows::ApplicationModel::WindowsAppRuntime::Common
{
    struct AutoInitialize
    {
        AutoInitialize()
        {
            Initialize();
        }

        ~AutoInitialize()
        {
#if MICROSOFT_WINDOWSAPPSDK_AUTOINITIALIZE_BOOTSTRAP
            Microsoft::Windows::ApplicationModel::DynamicDependency::Bootstrap::AutoInitialize::Shutdown();
#endif
        }

        static void Initialize()
        {
            // Call the AutoInitialize functions, as needed, starting with those initializing the WindowsAppRuntime
#if MICROSOFT_WINDOWSAPPSDK_AUTOINITIALIZE_BOOTSTRAP
            Microsoft::Windows::ApplicationModel::DynamicDependency::Bootstrap::AutoInitialize::Initialize();
#endif
#if MICROSOFT_WINDOWSAPPSDK_AUTOINITIALIZE_DEPLOYMENTMANAGER
            Microsoft::Windows::ApplicationModel::WindowsAppRuntime::DeploymentManager::AutoInitialize::Initialize();
#endif
#if MICROSOFT_WINDOWSAPPSDK_AUTOINITIALIZE_UNDOCKEDREGFREEWINRT
            Microsoft::Windows::Foundation::UndockedRegFreeWinRT::AutoInitialize::Initialize();
#endif

            // Compatibility happens just after initializing WindowsAppRuntime
#if MICROSOFT_WINDOWSAPPSDK_AUTOINITIALIZE_COMPATIBILITY
            Microsoft::Windows::ApplicationModel::WindowsAppRuntime::Compatibility::AutoInitialize::Initialize();
#endif
        }
    };

// Use init_seg(lib) to ensure this static object is initialized during the "lib" phase,
// which occurs before user-defined static objects. This guarantees that WindowsAppRuntime
// auto-initialization completes before any application static globals that might depend on it.
#pragma warning(disable : 4073)
#pragma init_seg(lib)
    static AutoInitialize g_autoInitialize;
#pragma warning(default : 4073)
}
