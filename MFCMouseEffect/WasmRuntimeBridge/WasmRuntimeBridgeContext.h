#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct M3Environment;
struct M3Runtime;
struct M3Function;
struct M3Module;

namespace wasm_runtime_bridge {

class RuntimeBridgeContext {
public:
    RuntimeBridgeContext();
    ~RuntimeBridgeContext();

    RuntimeBridgeContext(const RuntimeBridgeContext&) = delete;
    RuntimeBridgeContext& operator=(const RuntimeBridgeContext&) = delete;

    bool LoadModuleFromFile(const wchar_t* modulePath);
    void UnloadModule();
    bool IsModuleLoaded() const;

    bool CallGetApiVersion(uint32_t* outApiVersion);
    bool CallOnClick(
        const uint8_t* inputPtr,
        uint32_t inputLen,
        uint8_t* outputPtr,
        uint32_t outputCap,
        uint32_t* outWrittenBytes);

    void ResetPluginState();
    const char* LastError() const;

private:
    bool CreateRuntimeLocked();
    void ReleaseRuntimeLocked();
    bool LoadModuleFromCachedBytesLocked();
    bool LinkHostImportsLocked(M3Module* module);
    bool RecoverRuntimeAfterFaultLocked();
    bool ResolvePluginExportsLocked();
    bool EnsureScratchMemoryLocked(
        uint32_t inputLen,
        uint32_t outputCap,
        uint32_t* outInputOffset,
        uint32_t* outOutputOffset);
    void SetErrorLocked(const std::string& message);
    void SetErrorLocked(const char* message);
    void SetWasm3ErrorLocked(const char* prefix, const char* wasm3Result);
    void ClearErrorLocked();

private:
    mutable std::mutex mutex_{};
    std::string lastError_{};

    M3Environment* environment_ = nullptr;
    M3Runtime* runtime_ = nullptr;

    M3Function* fnGetApiVersion_ = nullptr;
    M3Function* fnOnClick_ = nullptr;
    M3Function* fnReset_ = nullptr;

    std::vector<uint8_t> wasmBytes_{};
    bool moduleLoaded_ = false;
};

std::unique_ptr<RuntimeBridgeContext> CreateRuntimeBridge();

} // namespace wasm_runtime_bridge

