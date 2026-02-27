#include "pch.h"

#include "SettingsStateMapper.EffectsProfileStateBuilder.h"

#include "MouseFx/Core/Config/EffectConfig.h"
#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "SettingsStateMapper.EffectsProfileStateBuilder.Macos.h"
#endif

namespace mousefx {

nlohmann::json BuildEffectsProfileStateJson(const EffectConfig& cfg) {
    nlohmann::json out = nlohmann::json::object();
    out["active"] = {
        {"click", cfg.active.click},
        {"trail", cfg.active.trail},
        {"scroll", cfg.active.scroll},
        {"hold", cfg.active.hold},
        {"hover", cfg.active.hover},
    };

#if MFX_PLATFORM_MACOS
    nlohmann::json macosOut = BuildMacosEffectsProfileStateJson(cfg);
    for (auto it = macosOut.begin(); it != macosOut.end(); ++it) {
        out[it.key()] = it.value();
    }
#else
    out["platform"] = "non_macos";
#endif

    return out;
}

} // namespace mousefx
