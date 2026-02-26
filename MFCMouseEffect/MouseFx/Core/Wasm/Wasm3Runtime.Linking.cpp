#include "pch.h"

#include "Wasm3Runtime.h"

#include "Wasm3Runtime.Internal.h"

#include "WasmRuntimeBridge/third_party/wasm3/source/m3_env.h"
#include "WasmRuntimeBridge/third_party/wasm3/source/wasm3.h"

namespace mousefx::wasm {
namespace {

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

} // namespace

bool Wasm3Runtime::LinkHostImports(M3Module* module, std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (!module) {
        SetOutError(outError, "module is null while linking host imports.");
        return false;
    }

    M3Result result = SuppressLookupFailure(
        m3_LinkRawFunction(module, "env", "abort", "v(iiii)", &HostAssemblyScriptAbort));
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_LinkRawFunction(env.abort) failed", result));
        return false;
    }

    result = SuppressLookupFailure(
        m3_LinkRawFunction(module, "env", "_abort", "v()", &HostAbortNoArgs));
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_LinkRawFunction(env._abort) failed", result));
        return false;
    }

    return true;
}

bool Wasm3Runtime::ResolvePluginExports(std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (!runtime_) {
        SetOutError(outError, "runtime is not initialized.");
        return false;
    }

    M3Result result = m3_FindFunction(&fnGetApiVersion_, runtime_, "mfx_plugin_get_api_version");
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_FindFunction(mfx_plugin_get_api_version) failed", result));
        return false;
    }

    result = m3_FindFunction(&fnOnEvent_, runtime_, "mfx_plugin_on_event");
    if (result) {
        fnOnEvent_ = nullptr;
    }
    if (!fnOnEvent_) {
        SetOutError(outError, "plugin does not export mfx_plugin_on_event.");
        return false;
    }

    result = m3_FindFunction(&fnReset_, runtime_, "mfx_plugin_reset");
    if (result) {
        fnReset_ = nullptr;
    }
    return true;
}

} // namespace mousefx::wasm
