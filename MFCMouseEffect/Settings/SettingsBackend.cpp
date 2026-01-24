#include "pch.h"
#include "SettingsBackend.h"

#include "../MouseFx/AppController.h"

namespace {

class AppControllerBackend final : public ISettingsBackend {
public:
    explicit AppControllerBackend(mousefx::AppController* c) : c_(c) {}

    SettingsModel Load() override {
        SettingsModel m;
        if (!c_) return m;

        const auto& cfg = c_->Config();
        m.uiLanguage = cfg.uiLanguage.empty() ? "zh-CN" : cfg.uiLanguage;
        m.theme = cfg.theme.empty() ? "neon" : cfg.theme;

        m.click = cfg.active.click.empty() ? "ripple" : cfg.active.click;
        m.trail = cfg.active.trail.empty() ? "particle" : cfg.active.trail;
        m.scroll = cfg.active.scroll.empty() ? "arrow" : cfg.active.scroll;
        m.hold = cfg.active.hold.empty() ? "charge" : cfg.active.hold;
        m.hover = cfg.active.hover.empty() ? "glow" : cfg.active.hover;
        return m;
    }

    void Apply(const SettingsModel& m) override {
        if (!c_) return;

        // Persisted setters.
        c_->SetUiLanguage(m.uiLanguage);
        c_->SetTheme(m.theme);

        auto applyEffect = [&](const char* category, const std::string& type) {
            if (type == "none") {
                std::string cmd = std::string("{\"cmd\":\"clear_effect\",\"category\":\"") + category + "\"}";
                c_->HandleCommand(cmd);
                return;
            }
            std::string cmd = std::string("{\"cmd\":\"set_effect\",\"category\":\"") + category +
                "\",\"type\":\"" + type + "\"}";
            c_->HandleCommand(cmd);
        };

        applyEffect("click", m.click);
        applyEffect("trail", m.trail);
        applyEffect("scroll", m.scroll);
        applyEffect("hold", m.hold);
        applyEffect("hover", m.hover);
    }

private:
    mousefx::AppController* c_ = nullptr;
};

} // namespace

std::unique_ptr<ISettingsBackend> CreateSettingsBackend(mousefx::AppController* controller) {
    return std::make_unique<AppControllerBackend>(controller);
}
