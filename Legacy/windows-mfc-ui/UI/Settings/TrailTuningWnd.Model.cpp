#include "pch.h"
#include "UI/Settings/TrailTuningWnd.h"

#include <cmath>

#include "Settings/SettingsBackend.h"

namespace {

static std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

static TrailTuningModel MakePresetModel(const std::string& preset) {
    TrailTuningModel m;
    std::string p = ToLowerAscii(preset);

    if (p == "snappy") {
        m.style = "snappy";
        m.profiles.line = {220, 24};
        m.profiles.streamer = {320, 36};
        m.profiles.electric = {220, 18};
        m.profiles.meteor = {380, 44};
        m.profiles.tubes = {280, 32};

        m.params.electricForkChance = 0.14f;
        m.params.electricAmplitudeScale = 1.15f;
        m.params.streamerGlowWidthScale = 1.6f;
        m.params.meteorSparkRateScale = 1.1f;
        return m;
    }
    if (p == "long") {
        m.style = "long";
        m.profiles.line = {380, 44};
        m.profiles.streamer = {520, 64};
        m.profiles.electric = {360, 36};
        m.profiles.meteor = {720, 90};
        m.profiles.tubes = {520, 56};

        m.params.streamerGlowWidthScale = 2.1f;
        m.params.streamerHeadPower = 1.4f;
        m.params.electricForkChance = 0.08f;
        m.params.meteorSparkRateScale = 1.25f;
        return m;
    }
    if (p == "cinematic") {
        m.style = "cinematic";
        m.profiles.line = {460, 58};
        m.profiles.streamer = {640, 88};
        m.profiles.electric = {380, 44};
        m.profiles.meteor = {900, 120};
        m.profiles.tubes = {620, 72};

        m.params.streamerGlowWidthScale = 2.4f;
        m.params.streamerHeadPower = 1.25f;
        m.params.electricForkChance = 0.06f;
        m.params.electricAmplitudeScale = 1.05f;
        m.params.meteorSparkRateScale = 1.45f;
        m.params.meteorSparkSpeedScale = 1.15f;
        return m;
    }

    m.style = "default";
    return m;
}

} // namespace

void CTrailTuningWnd::OnCommandApply() {
    if (!backend_) return;
    if (!SyncModelFromControls()) return;
    backend_->ApplyTrailTuning(model_);
}

void CTrailTuningWnd::OnCommandReset() {
    ApplyPreset("default");
    SyncControlsFromModel();
}

void CTrailTuningWnd::OnSelChangePreset() {
    int sel = cmbPreset_.GetCurSel();
    CString s;
    if (sel >= 0) cmbPreset_.GetLBText(sel, s);
    std::string preset;
    preset.reserve((size_t)s.GetLength());
    for (int i = 0; i < s.GetLength(); ++i) {
        wchar_t ch = s[i];
        if (ch >= L'A' && ch <= L'Z') ch = (wchar_t)(ch - L'A' + L'a');
        preset.push_back((ch >= 0 && ch <= 0x7F) ? (char)ch : '?');
    }
    ApplyPreset(preset);
    SyncControlsFromModel();
}

int CTrailTuningWnd::ParseInt(const CString& s, int fallback) {
    int v = fallback;
    if (swscanf_s(s, L"%d", &v) == 1) return v;
    return fallback;
}

float CTrailTuningWnd::ParseFloat(const CString& s, float fallback) {
    float v = fallback;
    if (swscanf_s(s, L"%f", &v) == 1) return v;
    return fallback;
}

int CTrailTuningWnd::ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float CTrailTuningWnd::ClampFloat(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void CTrailTuningWnd::LoadFromBackend() {
    if (!backend_) return;
    model_ = backend_->LoadTrailTuning();

    SettingsModel sm = backend_->Load();
    if (!sm.uiLanguage.empty() && sm.uiLanguage != uiLanguage_) {
        uiLanguage_ = sm.uiLanguage;
        ApplyLanguageToControls();
    }

    SyncControlsFromModel();

    std::string p = ToLowerAscii(model_.style);
    int sel = 0;
    if (p == "snappy") sel = 1;
    else if (p == "long") sel = 2;
    else if (p == "cinematic") sel = 3;
    else if (p == "custom") sel = 4;
    cmbPreset_.SetCurSel(sel);
}

void CTrailTuningWnd::ApplyPreset(const std::string& preset) {
    std::string p = ToLowerAscii(preset);
    if (p == "custom") {
        model_.style = "custom";
        return;
    }
    if (p == "default") {
        model_ = TrailTuningModel{};
        model_.style = "default";
        return;
    }
    model_ = MakePresetModel(p);
}

void CTrailTuningWnd::SyncControlsFromModel() {
    auto setProfile = [&](ProfileControls& pc, const TrailHistoryProfileModel& p) {
        CString a, b;
        a.Format(L"%d", p.durationMs);
        b.Format(L"%d", p.maxPoints);
        pc.duration.SetWindowTextW(a);
        pc.maxPoints.SetWindowTextW(b);
    };
    setProfile(line_, model_.profiles.line);
    setProfile(streamer_, model_.profiles.streamer);
    setProfile(electric_, model_.profiles.electric);
    setProfile(meteor_, model_.profiles.meteor);
    setProfile(tubes_, model_.profiles.tubes);

    auto setF = [&](CEdit& e, float v) {
        CString s;
        s.Format(L"%.3f", v);
        e.SetWindowTextW(s);
    };
    setF(edtStreamerGlow_, model_.params.streamerGlowWidthScale);
    setF(edtStreamerCore_, model_.params.streamerCoreWidthScale);
    setF(edtStreamerHead_, model_.params.streamerHeadPower);
    setF(edtElectricFork_, model_.params.electricForkChance);
    setF(edtElectricAmp_, model_.params.electricAmplitudeScale);
    setF(edtMeteorRate_, model_.params.meteorSparkRateScale);
    setF(edtMeteorSpeed_, model_.params.meteorSparkSpeedScale);
}

bool CTrailTuningWnd::SyncModelFromControls() {
    auto getText = [](CWnd& w) -> CString {
        CString s;
        w.GetWindowTextW(s);
        return s;
    };
    auto readProfile = [&](ProfileControls& pc, TrailHistoryProfileModel& p) {
        p.durationMs = ClampInt(ParseInt(getText(pc.duration), p.durationMs), 80, 2000);
        p.maxPoints = ClampInt(ParseInt(getText(pc.maxPoints), p.maxPoints), 2, 240);
    };

    readProfile(line_, model_.profiles.line);
    readProfile(streamer_, model_.profiles.streamer);
    readProfile(electric_, model_.profiles.electric);
    readProfile(meteor_, model_.profiles.meteor);
    readProfile(tubes_, model_.profiles.tubes);

    model_.params.streamerGlowWidthScale = ClampFloat(ParseFloat(getText(edtStreamerGlow_), model_.params.streamerGlowWidthScale), 0.5f, 4.0f);
    model_.params.streamerCoreWidthScale = ClampFloat(ParseFloat(getText(edtStreamerCore_), model_.params.streamerCoreWidthScale), 0.2f, 2.0f);
    model_.params.streamerHeadPower = ClampFloat(ParseFloat(getText(edtStreamerHead_), model_.params.streamerHeadPower), 0.8f, 3.0f);
    model_.params.electricForkChance = ClampFloat(ParseFloat(getText(edtElectricFork_), model_.params.electricForkChance), 0.0f, 0.5f);
    model_.params.electricAmplitudeScale = ClampFloat(ParseFloat(getText(edtElectricAmp_), model_.params.electricAmplitudeScale), 0.2f, 3.0f);
    model_.params.meteorSparkRateScale = ClampFloat(ParseFloat(getText(edtMeteorRate_), model_.params.meteorSparkRateScale), 0.2f, 4.0f);
    model_.params.meteorSparkSpeedScale = ClampFloat(ParseFloat(getText(edtMeteorSpeed_), model_.params.meteorSparkSpeedScale), 0.2f, 4.0f);

    int sel = cmbPreset_.GetCurSel();
    if (sel < 0) sel = 4;
    if (sel == 0) model_.style = "default";
    else if (sel == 1) model_.style = "snappy";
    else if (sel == 2) model_.style = "long";
    else if (sel == 3) model_.style = "cinematic";
    else model_.style = "custom";

    auto sameI = [](int a, int b) { return a == b; };
    auto sameF = [](float a, float b) { return std::fabs(a - b) < 0.0005f; };
    auto sameP = [&](const TrailHistoryProfileModel& a, const TrailHistoryProfileModel& b) {
        return sameI(a.durationMs, b.durationMs) && sameI(a.maxPoints, b.maxPoints);
    };

    if (model_.style != "custom") {
        TrailTuningModel preset = MakePresetModel(model_.style);
        bool same =
            sameP(model_.profiles.line, preset.profiles.line) &&
            sameP(model_.profiles.streamer, preset.profiles.streamer) &&
            sameP(model_.profiles.electric, preset.profiles.electric) &&
            sameP(model_.profiles.meteor, preset.profiles.meteor) &&
            sameP(model_.profiles.tubes, preset.profiles.tubes) &&
            sameF(model_.params.streamerGlowWidthScale, preset.params.streamerGlowWidthScale) &&
            sameF(model_.params.streamerCoreWidthScale, preset.params.streamerCoreWidthScale) &&
            sameF(model_.params.streamerHeadPower, preset.params.streamerHeadPower) &&
            sameF(model_.params.electricAmplitudeScale, preset.params.electricAmplitudeScale) &&
            sameF(model_.params.electricForkChance, preset.params.electricForkChance) &&
            sameF(model_.params.meteorSparkRateScale, preset.params.meteorSparkRateScale) &&
            sameF(model_.params.meteorSparkSpeedScale, preset.params.meteorSparkSpeedScale);
        if (!same) model_.style = "custom";
    }

    return true;
}

