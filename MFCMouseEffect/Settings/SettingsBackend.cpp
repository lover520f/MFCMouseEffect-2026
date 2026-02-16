#include "pch.h"
#include "SettingsBackend.h"

#include "../MouseFx/Core/Control/AppController.h"

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

        // Flatten texts to comma-separated UTF-8 string
        std::string s;
        for (size_t i = 0; i < cfg.textClick.texts.size(); ++i) {
            std::string utf8 = WStringToUtf8(cfg.textClick.texts[i]);
            if (i > 0) s += ",";
            s += utf8;
        }
        m.textContent = s;

        return m;
    }

    void Apply(const SettingsModel& m) override {
        if (!c_) return;

        // Persisted setters.
        c_->SetUiLanguage(m.uiLanguage);
        c_->SetTheme(m.theme);
        
        // Parse text content
        {
            std::vector<std::wstring> texts;
            std::string raw = m.textContent;
            size_t start = 0;
            size_t end = raw.find(',');
            while (end != std::string::npos) {
                std::string token = TrimAscii(raw.substr(start, end - start));
                if (!token.empty()) texts.push_back(Utf8ToWString(token));
                start = end + 1;
                end = raw.find(',', start);
            }
            std::string last = TrimAscii(raw.substr(start));
            if (!last.empty()) texts.push_back(Utf8ToWString(last));
            
            c_->SetTextEffectContent(texts);
        }

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

    void ResetToDefaults() override {
        if (c_) c_->ResetConfig();
    }

    TrailTuningModel LoadTrailTuning() override {
        TrailTuningModel out;
        if (!c_) return out;

        const auto& cfg = c_->Config();
        out.style = cfg.trailStyle.empty() ? "default" : cfg.trailStyle;

        auto toModel = [](const mousefx::TrailHistoryProfile& p) -> TrailHistoryProfileModel {
            return {p.durationMs, p.maxPoints};
        };
        out.profiles.line = toModel(cfg.trailProfiles.line);
        out.profiles.streamer = toModel(cfg.trailProfiles.streamer);
        out.profiles.electric = toModel(cfg.trailProfiles.electric);
        out.profiles.meteor = toModel(cfg.trailProfiles.meteor);
        out.profiles.tubes = toModel(cfg.trailProfiles.tubes);

        out.params.streamerGlowWidthScale = cfg.trailParams.streamer.glowWidthScale;
        out.params.streamerCoreWidthScale = cfg.trailParams.streamer.coreWidthScale;
        out.params.streamerHeadPower = cfg.trailParams.streamer.headPower;
        out.params.electricAmplitudeScale = cfg.trailParams.electric.amplitudeScale;
        out.params.electricForkChance = cfg.trailParams.electric.forkChance;
        out.params.meteorSparkRateScale = cfg.trailParams.meteor.sparkRateScale;
        out.params.meteorSparkSpeedScale = cfg.trailParams.meteor.sparkSpeedScale;

        return out;
    }

    void ApplyTrailTuning(const TrailTuningModel& tuning) override {
        if (!c_) return;

        mousefx::TrailProfilesConfig profiles;
        profiles.line = {tuning.profiles.line.durationMs, tuning.profiles.line.maxPoints};
        profiles.streamer = {tuning.profiles.streamer.durationMs, tuning.profiles.streamer.maxPoints};
        profiles.electric = {tuning.profiles.electric.durationMs, tuning.profiles.electric.maxPoints};
        profiles.meteor = {tuning.profiles.meteor.durationMs, tuning.profiles.meteor.maxPoints};
        profiles.tubes = {tuning.profiles.tubes.durationMs, tuning.profiles.tubes.maxPoints};

        mousefx::TrailRendererParamsConfig params;
        params.streamer.glowWidthScale = tuning.params.streamerGlowWidthScale;
        params.streamer.coreWidthScale = tuning.params.streamerCoreWidthScale;
        params.streamer.headPower = tuning.params.streamerHeadPower;
        params.electric.amplitudeScale = tuning.params.electricAmplitudeScale;
        params.electric.forkChance = tuning.params.electricForkChance;
        params.meteor.sparkRateScale = tuning.params.meteorSparkRateScale;
        params.meteor.sparkSpeedScale = tuning.params.meteorSparkSpeedScale;

        c_->SetTrailTuning(tuning.style, profiles, params);
    }

private:
    static std::string TrimAscii(std::string s) {
        auto is_space = [](unsigned char ch) {
            return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
        };
        size_t b = 0;
        while (b < s.size() && is_space((unsigned char)s[b])) b++;
        size_t e = s.size();
        while (e > b && is_space((unsigned char)s[e - 1])) e--;
        if (b == 0 && e == s.size()) return s;
        return s.substr(b, e - b);
    }
    static std::string WStringToUtf8(const std::wstring& ws) {
        if (ws.empty()) return {};
        int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) return {};
        std::string out(len - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, out.empty() ? nullptr : &out[0], len, nullptr, nullptr);
        return out;
    }

    static std::wstring Utf8ToWString(const std::string& s) {
        if (s.empty()) return {};
        int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        if (len <= 0) return {};
        std::wstring out(len - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &out[0], len);
        return out;
    }

    mousefx::AppController* c_ = nullptr;
};

} // namespace

std::unique_ptr<ISettingsBackend> CreateSettingsBackend(mousefx::AppController* controller) {
    return std::make_unique<AppControllerBackend>(controller);
}
