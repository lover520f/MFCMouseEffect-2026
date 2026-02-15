#include "pch.h"
#include "EffectConfig.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonCodec.h"

#include <sstream>

namespace mousefx {

EffectConfig EffectConfig::Load(const std::wstring& exeDir) {
    EffectConfig config = GetDefault();

    std::wstring configPath = exeDir + L"\\config.json";
#ifdef _DEBUG
    OutputDebugStringW((L"MouseFx: Loading config from " + configPath + L"\n").c_str());
#endif

    std::string jsonContent = config_internal::ReadFileAsUtf8(configPath);
    if (jsonContent.empty()) {
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: config.json not found or empty, creating default.\n");
#endif
        Save(exeDir, config);
        return config;
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(jsonContent);
    } catch (const nlohmann::json::exception& ex) {
#ifndef _DEBUG
        (void)ex;
#endif
#ifdef _DEBUG
        std::wstringstream ss;
        ss << L"MouseFx: JSON parse error: " << ex.what() << L". Recreating config.\n";
        OutputDebugStringW(ss.str().c_str());
#endif
        Save(exeDir, config);
        return config;
    }

    config_json::ApplyRootToConfig(root, config);

#ifdef _DEBUG
    OutputDebugStringW(L"MouseFx: config.json loaded successfully.\n");
#endif
    return config;
}

} // namespace mousefx
