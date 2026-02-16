#include "pch.h"
#include "EffectConfig.h"

#include "EffectConfigJsonCodec.h"

#include <fstream>

namespace mousefx {

bool EffectConfig::Save(const std::wstring& exeDir, const EffectConfig& config) {
    std::wstring configPath = exeDir + L"\\config.json";
    nlohmann::json root = config_json::BuildRootFromConfig(config);

    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    file << root.dump(2);
    return true;
}

} // namespace mousefx
