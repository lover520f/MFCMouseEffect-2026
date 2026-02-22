#include "WasmRuntimeBridgeContext.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <excpt.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

#include "third_party/wasm3/source/m3_env.h"
#include "third_party/wasm3/source/wasm3.h"

namespace wasm_runtime_bridge {

namespace {

constexpr uint32_t kRuntimeStackBytes = 256u * 1024u;
constexpr uint32_t kLinearMemoryLimitBytes = 4u * 1024u * 1024u;
constexpr uint32_t kWasmPageBytes = 64u * 1024u;
constexpr uint32_t kScratchAlignment = 16u;

uint32_t AlignUp(uint32_t value, uint32_t alignment) {
    if (alignment == 0) {
        return value;
    }
    const uint32_t remainder = value % alignment;
    return remainder == 0 ? value : (value + alignment - remainder);
}

std::string NarrowPathForLog(const std::filesystem::path& path) {
    return path.string();
}

std::string BuildSehErrorMessage(const char* stage, DWORD exceptionCode, bool recovered) {
    char buffer[192]{};
    const int written = std::snprintf(
        buffer,
        sizeof(buffer),
        "%s raised SEH 0x%08lX%s",
        (stage && stage[0] != '\0') ? stage : "runtime call",
        static_cast<unsigned long>(exceptionCode),
        recovered ? " (runtime rebuilt)" : "");
    if (written <= 0) {
        return recovered ? "runtime call raised SEH (runtime rebuilt)" : "runtime call raised SEH";
    }
    return std::string(buffer);
}

enum class ProtectedInvokeStatus : uint8_t {
    Ok = 0,
    SehFault,
};

M3Result SuppressLookupFailure(M3Result result) {
    return (result == m3Err_functionLookupFailed) ? m3Err_none : result;
}

m3ApiRawFunction(HostAbortNoArgs) {
    m3ApiTrap(m3Err_trapAbort);
}

m3ApiRawFunction(HostAssemblyScriptAbort) {
    m3ApiGetArg(uint32_t, messageOffset);
    m3ApiGetArg(uint32_t, fileOffset);
    m3ApiGetArg(uint32_t, lineNumber);
    m3ApiGetArg(uint32_t, columnNumber);
    (void)messageOffset;
    (void)fileOffset;
    (void)lineNumber;
    (void)columnNumber;
    m3ApiTrap(m3Err_trapAbort);
}

ProtectedInvokeStatus SafeM3Call(
    IM3Function function,
    uint32_t argc,
    const void* args[],
    M3Result* outResult,
    DWORD* outSehCode) {
    if (outResult) {
        *outResult = m3Err_none;
    }
    if (outSehCode) {
        *outSehCode = 0;
    }
    __try {
        if (outResult) {
            *outResult = m3_Call(function, argc, args);
        } else {
            (void)m3_Call(function, argc, args);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        if (outSehCode) {
            *outSehCode = static_cast<DWORD>(GetExceptionCode());
        }
        return ProtectedInvokeStatus::SehFault;
    }
    return ProtectedInvokeStatus::Ok;
}

ProtectedInvokeStatus SafeM3GetResults(
    IM3Function function,
    uint32_t retc,
    const void* retPointers[],
    M3Result* outResult,
    DWORD* outSehCode) {
    if (outResult) {
        *outResult = m3Err_none;
    }
    if (outSehCode) {
        *outSehCode = 0;
    }
    __try {
        if (outResult) {
            *outResult = m3_GetResults(function, retc, retPointers);
        } else {
            (void)m3_GetResults(function, retc, retPointers);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        if (outSehCode) {
            *outSehCode = static_cast<DWORD>(GetExceptionCode());
        }
        return ProtectedInvokeStatus::SehFault;
    }
    return ProtectedInvokeStatus::Ok;
}

} // namespace

RuntimeBridgeContext::RuntimeBridgeContext() = default;

RuntimeBridgeContext::~RuntimeBridgeContext() {
    std::lock_guard<std::mutex> lock(mutex_);
    ReleaseRuntimeLocked();
    if (environment_) {
        m3_FreeEnvironment(environment_);
        environment_ = nullptr;
    }
}

bool RuntimeBridgeContext::LoadModuleFromFile(const wchar_t* modulePath) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!modulePath || modulePath[0] == L'\0') {
        SetErrorLocked("module path is empty.");
        return false;
    }

    const std::filesystem::path path(modulePath);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        SetErrorLocked(std::string("module file does not exist: ") + NarrowPathForLog(path));
        return false;
    }
    if (!std::filesystem::is_regular_file(path, ec) || ec) {
        SetErrorLocked(std::string("module path is not a file: ") + NarrowPathForLog(path));
        return false;
    }

    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        SetErrorLocked(std::string("cannot open module file: ") + NarrowPathForLog(path));
        return false;
    }
    const std::streamoff fileSize = input.tellg();
    if (fileSize <= 0) {
        SetErrorLocked("module file is empty.");
        return false;
    }
    if (static_cast<uint64_t>(fileSize) > static_cast<uint64_t>((std::numeric_limits<uint32_t>::max)())) {
        SetErrorLocked("module file is too large.");
        return false;
    }

    wasmBytes_.assign(static_cast<size_t>(fileSize), 0);
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(wasmBytes_.data()), fileSize);
    if (!input) {
        SetErrorLocked("failed to read module bytes.");
        wasmBytes_.clear();
        return false;
    }

    ReleaseRuntimeLocked();
    if (!CreateRuntimeLocked()) {
        return false;
    }
    if (!LoadModuleFromCachedBytesLocked()) {
        ReleaseRuntimeLocked();
        return false;
    }
    return true;
}

void RuntimeBridgeContext::UnloadModule() {
    std::lock_guard<std::mutex> lock(mutex_);
    ReleaseRuntimeLocked();
    CreateRuntimeLocked();
}

bool RuntimeBridgeContext::IsModuleLoaded() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return moduleLoaded_ && runtime_ && fnGetApiVersion_ && fnOnEvent_;
}

bool RuntimeBridgeContext::CallGetApiVersion(uint32_t* outApiVersion) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (outApiVersion) {
        *outApiVersion = 0;
    }
    if (!moduleLoaded_ || !runtime_ || !fnGetApiVersion_) {
        SetErrorLocked("plugin module is not loaded.");
        return false;
    }

    M3Result result = m3Err_none;
    DWORD sehCode = 0;
    if (SafeM3Call(fnGetApiVersion_, 0, nullptr, &result, &sehCode) == ProtectedInvokeStatus::SehFault) {
        const bool recovered = RecoverRuntimeAfterFaultLocked();
        SetErrorLocked(BuildSehErrorMessage("m3_Call(get_api_version)", sehCode, recovered));
        return false;
    }
    if (result) {
        SetWasm3ErrorLocked("m3_Call(get_api_version) failed", result);
        return false;
    }

    uint32_t apiVersion = 0;
    const void* retPointers[1] = { &apiVersion };
    sehCode = 0;
    if (SafeM3GetResults(fnGetApiVersion_, 1, retPointers, &result, &sehCode) == ProtectedInvokeStatus::SehFault) {
        const bool recovered = RecoverRuntimeAfterFaultLocked();
        SetErrorLocked(BuildSehErrorMessage("m3_GetResults(get_api_version)", sehCode, recovered));
        return false;
    }
    if (result) {
        SetWasm3ErrorLocked("m3_GetResults(get_api_version) failed", result);
        return false;
    }

    if (outApiVersion) {
        *outApiVersion = apiVersion;
    }
    ClearErrorLocked();
    return true;
}

bool RuntimeBridgeContext::CallOnEvent(
    const uint8_t* inputPtr,
    uint32_t inputLen,
    uint8_t* outputPtr,
    uint32_t outputCap,
    uint32_t* outWrittenBytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!moduleLoaded_ || !runtime_ || !fnOnEvent_) {
        if (outWrittenBytes) {
            *outWrittenBytes = 0;
        }
        SetErrorLocked("plugin module does not export mfx_plugin_on_event.");
        return false;
    }
    return CallPluginBufferFunctionLocked(
        fnOnEvent_,
        "on_event",
        inputPtr,
        inputLen,
        outputPtr,
        outputCap,
        outWrittenBytes);
}

bool RuntimeBridgeContext::CallPluginBufferFunctionLocked(
    M3Function* function,
    const char* stageTag,
    const uint8_t* inputPtr,
    uint32_t inputLen,
    uint8_t* outputPtr,
    uint32_t outputCap,
    uint32_t* outWrittenBytes) {
    if (outWrittenBytes) {
        *outWrittenBytes = 0;
    }
    if (!moduleLoaded_ || !runtime_ || !function) {
        SetErrorLocked("plugin module is not loaded.");
        return false;
    }
    if (inputLen > 0 && !inputPtr) {
        SetErrorLocked("input pointer is null.");
        return false;
    }
    if (outputCap > 0 && !outputPtr) {
        SetErrorLocked("output pointer is null.");
        return false;
    }

    uint32_t inputOffset = 0;
    uint32_t outputOffset = 0;
    if (!EnsureScratchMemoryLocked(inputLen, outputCap, &inputOffset, &outputOffset)) {
        return false;
    }

    uint32_t memorySize = 0;
    uint8_t* memory = m3_GetMemory(runtime_, &memorySize, 0);
    if (!memory) {
        SetErrorLocked("wasm linear memory is unavailable.");
        return false;
    }
    if (inputLen > 0) {
        std::memcpy(memory + inputOffset, inputPtr, inputLen);
    }

    const uint32_t argInputOffset = inputOffset;
    const uint32_t argInputLen = inputLen;
    const uint32_t argOutputOffset = outputOffset;
    const uint32_t argOutputCap = outputCap;
    const void* argPointers[4] = {
        &argInputOffset,
        &argInputLen,
        &argOutputOffset,
        &argOutputCap,
    };

    M3Result result = m3Err_none;
    DWORD sehCode = 0;
    std::string stageCall = "m3_Call(";
    stageCall += (stageTag && stageTag[0] != '\0') ? stageTag : "plugin";
    stageCall += ")";
    if (SafeM3Call(function, 4, argPointers, &result, &sehCode) == ProtectedInvokeStatus::SehFault) {
        const bool recovered = RecoverRuntimeAfterFaultLocked();
        SetErrorLocked(BuildSehErrorMessage(stageCall.c_str(), sehCode, recovered));
        return false;
    }
    if (result) {
        std::string errorPrefix = stageCall + " failed";
        SetWasm3ErrorLocked(errorPrefix.c_str(), result);
        return false;
    }

    uint32_t writtenBytes = 0;
    const void* retPointers[1] = { &writtenBytes };
    sehCode = 0;
    std::string stageGetResults = "m3_GetResults(";
    stageGetResults += (stageTag && stageTag[0] != '\0') ? stageTag : "plugin";
    stageGetResults += ")";
    if (SafeM3GetResults(function, 1, retPointers, &result, &sehCode) == ProtectedInvokeStatus::SehFault) {
        const bool recovered = RecoverRuntimeAfterFaultLocked();
        SetErrorLocked(BuildSehErrorMessage(stageGetResults.c_str(), sehCode, recovered));
        return false;
    }
    if (result) {
        std::string errorPrefix = stageGetResults + " failed";
        SetWasm3ErrorLocked(errorPrefix.c_str(), result);
        return false;
    }
    if (writtenBytes > outputCap) {
        SetErrorLocked("plugin returned bytes larger than output capacity.");
        return false;
    }

    if (writtenBytes > 0) {
        if (!outputPtr) {
            SetErrorLocked("output pointer is null while plugin returned data.");
            return false;
        }
        uint32_t memorySizeAfterCall = 0;
        uint8_t* memoryAfterCall = m3_GetMemory(runtime_, &memorySizeAfterCall, 0);
        if (!memoryAfterCall) {
            SetErrorLocked("wasm linear memory is unavailable after plugin call.");
            return false;
        }
        const uint64_t outputEnd = static_cast<uint64_t>(outputOffset) + static_cast<uint64_t>(writtenBytes);
        if (outputEnd > static_cast<uint64_t>(memorySizeAfterCall)) {
            SetErrorLocked("plugin output range exceeds wasm linear memory.");
            return false;
        }
        std::memcpy(outputPtr, memoryAfterCall + outputOffset, writtenBytes);
    }

    if (outWrittenBytes) {
        *outWrittenBytes = writtenBytes;
    }
    ClearErrorLocked();
    return true;
}

void RuntimeBridgeContext::ResetPluginState() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!moduleLoaded_ || !runtime_ || !fnReset_) {
        return;
    }

    M3Result result = m3Err_none;
    DWORD sehCode = 0;
    if (SafeM3Call(fnReset_, 0, nullptr, &result, &sehCode) == ProtectedInvokeStatus::SehFault) {
        const bool recovered = RecoverRuntimeAfterFaultLocked();
        SetErrorLocked(BuildSehErrorMessage("m3_Call(reset)", sehCode, recovered));
        return;
    }
    if (result) {
        SetWasm3ErrorLocked("m3_Call(reset) failed", result);
        return;
    }
    ClearErrorLocked();
}

const char* RuntimeBridgeContext::LastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_.c_str();
}

bool RuntimeBridgeContext::CreateRuntimeLocked() {
    if (!environment_) {
        environment_ = m3_NewEnvironment();
        if (!environment_) {
            SetErrorLocked("m3_NewEnvironment failed.");
            return false;
        }
    }
    if (runtime_) {
        return true;
    }

    runtime_ = m3_NewRuntime(environment_, kRuntimeStackBytes, nullptr);
    if (!runtime_) {
        SetErrorLocked("m3_NewRuntime failed.");
        return false;
    }
    runtime_->memoryLimit = kLinearMemoryLimitBytes;
    return true;
}

bool RuntimeBridgeContext::LoadModuleFromCachedBytesLocked() {
    if (!runtime_ || !environment_) {
        SetErrorLocked("runtime is not initialized.");
        return false;
    }
    if (wasmBytes_.empty()) {
        SetErrorLocked("module bytes are empty.");
        return false;
    }

    IM3Module module = nullptr;
    M3Result result = m3_ParseModule(
        environment_,
        &module,
        wasmBytes_.data(),
        static_cast<uint32_t>(wasmBytes_.size()));
    if (result) {
        SetWasm3ErrorLocked("m3_ParseModule failed", result);
        if (module) {
            m3_FreeModule(module);
        }
        return false;
    }

    result = m3_LoadModule(runtime_, module);
    if (result) {
        SetWasm3ErrorLocked("m3_LoadModule failed", result);
        if (module) {
            m3_FreeModule(module);
        }
        return false;
    }

    if (!LinkHostImportsLocked(module)) {
        return false;
    }

    if (!ResolvePluginExportsLocked()) {
        return false;
    }

    moduleLoaded_ = true;
    ClearErrorLocked();
    return true;
}

bool RuntimeBridgeContext::RecoverRuntimeAfterFaultLocked() {
    const bool hasCachedModuleBytes = !wasmBytes_.empty();
    ReleaseRuntimeLocked();
    if (!hasCachedModuleBytes) {
        return false;
    }
    if (!CreateRuntimeLocked()) {
        return false;
    }
    if (!LoadModuleFromCachedBytesLocked()) {
        ReleaseRuntimeLocked();
        return false;
    }
    return true;
}

bool RuntimeBridgeContext::LinkHostImportsLocked(M3Module* module) {
    if (!module) {
        SetErrorLocked("module is null while linking host imports.");
        return false;
    }

    M3Result result = SuppressLookupFailure(
        m3_LinkRawFunction(module, "env", "abort", "v(iiii)", &HostAssemblyScriptAbort));
    if (result) {
        SetWasm3ErrorLocked("m3_LinkRawFunction(env.abort) failed", result);
        return false;
    }

    result = SuppressLookupFailure(
        m3_LinkRawFunction(module, "env", "_abort", "v()", &HostAbortNoArgs));
    if (result) {
        SetWasm3ErrorLocked("m3_LinkRawFunction(env._abort) failed", result);
        return false;
    }

    return true;
}

void RuntimeBridgeContext::ReleaseRuntimeLocked() {
    moduleLoaded_ = false;
    fnGetApiVersion_ = nullptr;
    fnOnEvent_ = nullptr;
    fnReset_ = nullptr;

    if (runtime_) {
        m3_FreeRuntime(runtime_);
        runtime_ = nullptr;
    }
}

bool RuntimeBridgeContext::ResolvePluginExportsLocked() {
    if (!runtime_) {
        SetErrorLocked("runtime is not initialized.");
        return false;
    }

    M3Result result = m3_FindFunction(&fnGetApiVersion_, runtime_, "mfx_plugin_get_api_version");
    if (result) {
        SetWasm3ErrorLocked("m3_FindFunction(mfx_plugin_get_api_version) failed", result);
        return false;
    }

    result = m3_FindFunction(&fnOnEvent_, runtime_, "mfx_plugin_on_event");
    if (result) {
        fnOnEvent_ = nullptr;
    }

    if (!fnOnEvent_) {
        SetErrorLocked("plugin does not export mfx_plugin_on_event.");
        return false;
    }

    result = m3_FindFunction(&fnReset_, runtime_, "mfx_plugin_reset");
    if (result) {
        fnReset_ = nullptr;
    }
    return true;
}

bool RuntimeBridgeContext::EnsureScratchMemoryLocked(
    uint32_t inputLen,
    uint32_t outputCap,
    uint32_t* outInputOffset,
    uint32_t* outOutputOffset) {
    if (!runtime_ || !outInputOffset || !outOutputOffset) {
        SetErrorLocked("runtime scratch preparation failed.");
        return false;
    }

    const uint32_t inputOffset = 0;
    const uint32_t outputOffset = AlignUp(inputLen, kScratchAlignment);
    const uint64_t requiredBytes = static_cast<uint64_t>(outputOffset) + static_cast<uint64_t>(outputCap);
    if (requiredBytes > static_cast<uint64_t>((std::numeric_limits<uint32_t>::max)())) {
        SetErrorLocked("scratch memory request exceeds 32-bit addressable range.");
        return false;
    }

    uint32_t currentBytes = m3_GetMemorySize(runtime_);
    if (currentBytes < requiredBytes) {
        const uint32_t requiredPages = static_cast<uint32_t>((requiredBytes + kWasmPageBytes - 1u) / kWasmPageBytes);
        M3Result resizeResult = ResizeMemory(runtime_, requiredPages);
        if (resizeResult) {
            SetWasm3ErrorLocked("ResizeMemory failed", resizeResult);
            return false;
        }
        currentBytes = m3_GetMemorySize(runtime_);
    }

    uint32_t memorySize = 0;
    uint8_t* memory = m3_GetMemory(runtime_, &memorySize, 0);
    if (!memory) {
        SetErrorLocked("wasm memory allocation returned null.");
        return false;
    }
    if (memorySize < requiredBytes) {
        SetErrorLocked("wasm memory is smaller than requested scratch range.");
        return false;
    }

    *outInputOffset = inputOffset;
    *outOutputOffset = outputOffset;
    return true;
}

void RuntimeBridgeContext::SetErrorLocked(const std::string& message) {
    lastError_ = message;
}

void RuntimeBridgeContext::SetErrorLocked(const char* message) {
    lastError_ = message ? message : "unknown runtime bridge error";
}

void RuntimeBridgeContext::SetWasm3ErrorLocked(const char* prefix, const char* wasm3Result) {
    lastError_.clear();
    if (prefix && prefix[0] != '\0') {
        lastError_.append(prefix);
        lastError_.append(": ");
    }
    lastError_.append(wasm3Result ? wasm3Result : "unknown wasm3 error");
}

void RuntimeBridgeContext::ClearErrorLocked() {
    lastError_.clear();
}

std::unique_ptr<RuntimeBridgeContext> CreateRuntimeBridge() {
    return std::make_unique<RuntimeBridgeContext>();
}

} // namespace wasm_runtime_bridge
