# Hybrid File Picker Sample (PartialTrust + Optional FullTrust Broker)

Related discussion: https://github.com/DinahK-2SO/WindowsAppSDK/discussions/10

## Rationale

This sample demonstrates a progressive enhancement strategy for file picking in WinUI 3 applications running under partial trust. It addresses current limitations where:

- UWP pickers (`Windows.Storage.Pickers`) don't work in elevated processes
- New `Microsoft.Windows.Storage.Pickers` APIs require full trust or specific capabilities
- WMS (Windows.Management.Storage) capability is not public
- Win32 Isolation is in preview and doesn't yet support partialTrust scenarios

The hybrid approach provides a fallback chain that works today while being extensible for future platform improvements.

## Architecture

The sample implements a strategy pattern with three file picker strategies:

### 1. NativePickerStrategy
- **When**: Full trust available or future Win32 Isolation supports partialTrust
- **How**: Direct use of `Microsoft.Windows.Storage.Pickers` APIs
- **Status**: Placeholder implementation (would use actual picker APIs when environment supports)

### 2. BrokerPickerStrategy  
- **When**: PartialTrust app with separate FullTrust broker available
- **How**: AppService IPC to communicate with broker process that has file system access
- **Implementation**: Uses `com.example.filepickerbroker` AppService for file picker operations

### 3. PassiveFallbackStrategy
- **When**: No elevated access available
- **How**: Provides guidance for drag & drop or manual file operations
- **Implementation**: Returns empty results with user guidance

### Strategy Resolution
The `FilePickerStrategyResolver` automatically selects the best available strategy:
1. Check for full trust indicator → `NativePickerStrategy`
2. Check if broker AppService available → `BrokerPickerStrategy`  
3. Fallback → `PassiveFallbackStrategy`

## Projects

### HybridFilePicker.App
- **Type**: WinUI 3 packaged app (partial trust)
- **Purpose**: Main application demonstrating hybrid file picker usage
- **Key Components**:
  - `IFilePickerService` - Service abstraction
  - Strategy implementations and resolver
  - Simple UI for testing file picker functionality

### HybridFilePicker.Broker
- **Type**: FullTrust console app with AppService
- **Purpose**: Provides file system access via IPC for partial trust apps
- **Key Components**:
  - `BrokerService` - AppService background task
  - File picker operations with result serialization
  - Supports both inline content (small files) and temp paths (large files)

## How It Works

### Typical Flow
1. User clicks "Pick Files" in the app
2. `FilePickerStrategyResolver` determines best available strategy
3. Selected strategy executes:
   - **Native**: Direct picker API call (when supported)
   - **Broker**: AppService request → broker launches picker → results returned via IPC
   - **Passive**: Returns empty with guidance message

### Broker Communication
- Uses Windows AppService (`com.example.filepickerbroker`)
- JSON serialization for file metadata
- Base64 encoding for small file content
- Temp file paths for larger files

### File Result Format
```csharp
public record PickedFile(
    string Id,              // Unique identifier
    string DisplayName,     // User-friendly filename
    string SuggestedExtension, // File extension
    byte[]? InlineContent,  // Small files as byte array
    string? TempPath        // Large files as temp path
);
```

## Building & Running

### Prerequisites
- Visual Studio 2022 with WinUI 3 support
- Windows App SDK
- Windows 10 1903+ or Windows 11

### Setup
1. Build the solution: `HybridFilePickerSample.sln`
2. For broker functionality:
   - Register the AppService in your app package manifest
   - Deploy both App and Broker projects
   - Ensure broker runs with full trust

### Manifest Notes
The broker requires AppService registration in Package.appxmanifest:
```xml
<Extensions>
  <uap:Extension Category="windows.appService">
    <uap:AppService Name="com.example.filepickerbroker" />
  </uap:Extension>
  <uap:Extension Category="windows.backgroundTasks" EntryPoint="HybridFilePicker.Broker.BrokerService">
    <uap:BackgroundTasks>
      <uap:Task Type="appService" />
    </uap:BackgroundTasks>
  </uap:Extension>
</Extensions>
```

### Limitations
- Current implementation uses placeholder picker logic for safety
- Real implementation would replace placeholders with actual `Microsoft.Windows.Storage.Pickers` calls
- Large file support via temp paths needs additional security considerations
- AppService communication limited by platform constraints

## Future Work

### Short Term
- Replace placeholder picker logic with actual `Microsoft.Windows.Storage.Pickers` implementation
- Add Named Pipes support for large file streaming
- Implement proper error handling and retry logic
- Add security validation for broker communication

### Long Term 
- Integrate with Win32 Isolation when GA supports partialTrust scenarios
- Remove broker dependency when platform provides native partialTrust picker support
- Add support for folder picker and save picker scenarios
- Implement capability-based detection for strategy selection

### Security Hardening
- Validate broker identity and signatures
- Implement secure file transfer mechanisms
- Add rate limiting and resource management
- Audit and log file access operations

## Extension Points

This sample is designed for easy evolution:

1. **Replace NativePickerStrategy**: Update when full trust or Win32 Isolation becomes available
2. **Enhance BrokerStrategy**: Add Named Pipes, security, or performance optimizations  
3. **Improve PassiveStrategy**: Add better drag & drop integration
4. **Custom Strategies**: Implement `IFilePickerService` for domain-specific needs

The abstraction allows seamless migration between approaches as the platform evolves.