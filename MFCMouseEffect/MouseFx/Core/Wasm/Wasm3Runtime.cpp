#include "pch.h"

#include "Wasm3Runtime.h"

#include "Wasm3Runtime.Internal.h"

#include <filesystem>
#include <fstream>
#include <limits>

#include "WasmRuntimeBridge/third_party/wasm3/source/m3_env.h"
#include "WasmRuntimeBridge/third_party/wasm3/source/wasm3.h"

namespace mousefx::wasm {
namespace {

void ReadModuleFileBytes(
    const std::wstring& modulePath,
    std::vector<uint8_t>* outBytes,
    std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (!outBytes) {
        SetOutError(outError, "module output buffer is null.");
        return;
    }
    if (modulePath.empty()) {
        SetOutError(outError, "module path is empty.");
        return;
    }

    const std::filesystem::path path(modulePath);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        SetOutError(outError, std::string("module file does not exist: ") + path.string());
        return;
    }
    if (!std::filesystem::is_regular_file(path, ec) || ec) {
        SetOutError(outError, std::string("module path is not a file: ") + path.string());
        return;
    }

    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input.is_open()) {
        SetOutError(outError, std::string("cannot open module file: ") + path.string());
        return;
    }

    const std::streamoff fileSize = input.tellg();
    if (fileSize <= 0) {
        SetOutError(outError, "module file is empty.");
        return;
    }
    if (static_cast<uint64_t>(fileSize) > static_cast<uint64_t>((std::numeric_limits<uint32_t>::max)())) {
        SetOutError(outError, "module file is too large.");
        return;
    }

    outBytes->assign(static_cast<size_t>(fileSize), 0);
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(outBytes->data()), fileSize);
    if (!input.good()) {
        SetOutError(outError, "failed to read module bytes.");
        outBytes->clear();
        return;
    }
    SetOutError(outError, "");
}

} // namespace

Wasm3Runtime::Wasm3Runtime() = default;

Wasm3Runtime::~Wasm3Runtime() {
    ReleaseRuntime();
    if (environment_) {
        m3_FreeEnvironment(environment_);
        environment_ = nullptr;
    }
}

bool Wasm3Runtime::LoadModuleFromFile(const std::wstring& modulePath, std::string* outError) {
    using namespace wasm3_runtime_detail;
    std::vector<uint8_t> wasmBytes;
    ReadModuleFileBytes(modulePath, &wasmBytes, outError);
    if (wasmBytes.empty()) {
        return false;
    }

    ReleaseRuntime();
    if (!CreateRuntime(outError)) {
        return false;
    }
    moduleBytes_ = std::move(wasmBytes);
    if (!LoadModuleFromCachedBytes(outError)) {
        ReleaseRuntime();
        return false;
    }

    SetOutError(outError, "");
    return true;
}

void Wasm3Runtime::UnloadModule() {
    ReleaseRuntime();
    (void)CreateRuntime(nullptr);
}

bool Wasm3Runtime::IsModuleLoaded() const {
    return moduleLoaded_ && runtime_ && fnGetApiVersion_ && fnOnEvent_;
}

bool Wasm3Runtime::CreateRuntime(std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (!environment_) {
        environment_ = m3_NewEnvironment();
        if (!environment_) {
            SetOutError(outError, "m3_NewEnvironment failed.");
            return false;
        }
    }
    if (runtime_) {
        SetOutError(outError, "");
        return true;
    }

    runtime_ = m3_NewRuntime(environment_, kRuntimeStackBytes, nullptr);
    if (!runtime_) {
        SetOutError(outError, "m3_NewRuntime failed.");
        return false;
    }
    runtime_->memoryLimit = kLinearMemoryLimitBytes;
    SetOutError(outError, "");
    return true;
}

void Wasm3Runtime::ReleaseRuntime() {
    moduleLoaded_ = false;
    fnGetApiVersion_ = nullptr;
    fnOnEvent_ = nullptr;
    fnReset_ = nullptr;
    moduleBytes_.clear();
    if (runtime_) {
        m3_FreeRuntime(runtime_);
        runtime_ = nullptr;
    }
}

bool Wasm3Runtime::LoadModuleFromCachedBytes(std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (!runtime_ || !environment_) {
        SetOutError(outError, "runtime is not initialized.");
        return false;
    }
    if (moduleBytes_.empty()) {
        SetOutError(outError, "module bytes are empty.");
        return false;
    }

    IM3Module module = nullptr;
    M3Result result = m3_ParseModule(
        environment_,
        &module,
        moduleBytes_.data(),
        static_cast<uint32_t>(moduleBytes_.size()));
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_ParseModule failed", result));
        if (module) {
            m3_FreeModule(module);
        }
        return false;
    }

    result = m3_LoadModule(runtime_, module);
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_LoadModule failed", result));
        if (module) {
            m3_FreeModule(module);
        }
        return false;
    }

    if (!LinkHostImports(module, outError)) {
        return false;
    }
    if (!ResolvePluginExports(outError)) {
        return false;
    }

    moduleLoaded_ = true;
    SetOutError(outError, "");
    return true;
}

} // namespace mousefx::wasm
