#pragma once

#include <cstddef>
#include <string>

struct SettingOption {
    const wchar_t* display;
    const char* value;
};

struct SettingsText {
    const wchar_t* title;
    const wchar_t* subtitle;

    const wchar_t* sectionGeneral;
    const wchar_t* sectionEffects;

    const wchar_t* labelLanguage;
    const wchar_t* labelTheme;

    const wchar_t* labelClick;
    const wchar_t* labelTrail;
    const wchar_t* labelScroll;
    const wchar_t* labelHold;
    const wchar_t* labelHover;

    const wchar_t* btnClose;
    const wchar_t* btnApply;
};

inline const SettingsText& TextZh() {
    static const SettingsText t{
        L"MFCMouseEffect \u8BBE\u7F6E",
        L"\u8F7B\u91CF\u914D\u7F6E\u7A97\u53E3\uFF0C\u4EC5\u975E background \u6A21\u5F0F\u53EF\u7528",
        L"\u4E00\u822C",
        L"\u7279\u6548",
        L"\u8BED\u8A00",
        L"\u4E3B\u9898",
        L"\u70B9\u51FB",
        L"\u62D6\u5C3E",
        L"\u6EDA\u8F6E",
        L"\u957F\u6309",
        L"\u60AC\u505C",
        L"\u5173\u95ED",
        L"\u5E94\u7528",
    };
    return t;
}

inline const SettingsText& TextEn() {
    static const SettingsText t{
        L"MFCMouseEffect Settings",
        L"Lightweight UI (non-background mode only)",
        L"General",
        L"Effects",
        L"Language",
        L"Theme",
        L"Click",
        L"Trail",
        L"Scroll",
        L"Hold",
        L"Hover",
        L"Close",
        L"Apply",
    };
    return t;
}

inline const SettingOption* LangOptions(size_t& n) {
    static const SettingOption opts[] = {
        {L"\u4E2D\u6587", "zh-CN"},
        {L"English", "en-US"},
    };
    n = _countof(opts);
    return opts;
}

inline const SettingOption* ThemeOptions(bool zh, size_t& n) {
    static const SettingOption zhOpts[] = {
        {L"\u9724\u8679", "neon"},
        {L"\u79D1\u5E7B", "scifi"},
        {L"\u6781\u7B80", "minimal"},
        {L"\u6E38\u620F\u611F", "game"},
    };
    static const SettingOption enOpts[] = {
        {L"Neon", "neon"},
        {L"Sci-Fi", "scifi"},
        {L"Minimal", "minimal"},
        {L"Game", "game"},
    };
    if (zh) {
        n = _countof(zhOpts);
        return zhOpts;
    }
    n = _countof(enOpts);
    return enOpts;
}

inline const SettingOption* ClickOptions(bool zh, size_t& n) {
    static const SettingOption zhOpts[] = {
        {L"\u6C34\u6CE2\u7EB9", "ripple"},
        {L"\u661F\u661F", "star"},
        {L"\u98D8\u6D6E\u6587\u5B57", "text"},
        {L"\u65E0", "none"},
    };
    static const SettingOption enOpts[] = {
        {L"Ripple", "ripple"},
        {L"Star", "star"},
        {L"Text", "text"},
        {L"None", "none"},
    };
    if (zh) {
        n = _countof(zhOpts);
        return zhOpts;
    }
    n = _countof(enOpts);
    return enOpts;
}

inline const SettingOption* TrailOptions(bool zh, size_t& n) {
    static const SettingOption zhOpts[] = {
        {L"\u5F69\u8679\u7C92\u5B50", "particle"},
        {L"\u666E\u901A\u7EBF\u6761", "line"},
        {L"\u65E0", "none"},
    };
    static const SettingOption enOpts[] = {
        {L"Particle", "particle"},
        {L"Line", "line"},
        {L"None", "none"},
    };
    if (zh) {
        n = _countof(zhOpts);
        return zhOpts;
    }
    n = _countof(enOpts);
    return enOpts;
}

inline const SettingOption* ScrollOptions(bool zh, size_t& n) {
    static const SettingOption zhOpts[] = {
        {L"\u65B9\u5411\u6307\u793A", "arrow"},
        {L"\u65E0", "none"},
    };
    static const SettingOption enOpts[] = {
        {L"Arrow", "arrow"},
        {L"None", "none"},
    };
    if (zh) {
        n = _countof(zhOpts);
        return zhOpts;
    }
    n = _countof(enOpts);
    return enOpts;
}

inline const SettingOption* HoldOptions(bool zh, size_t& n) {
    static const SettingOption zhOpts[] = {
        {L"\u84C4\u529B", "charge"},
        {L"\u65E0", "none"},
    };
    static const SettingOption enOpts[] = {
        {L"Charge", "charge"},
        {L"None", "none"},
    };
    if (zh) {
        n = _countof(zhOpts);
        return zhOpts;
    }
    n = _countof(enOpts);
    return enOpts;
}

inline const SettingOption* HoverOptions(bool zh, size_t& n) {
    static const SettingOption zhOpts[] = {
        {L"\u547C\u5438\u706F", "glow"},
        {L"\u65E0", "none"},
    };
    static const SettingOption enOpts[] = {
        {L"Glow", "glow"},
        {L"None", "none"},
    };
    if (zh) {
        n = _countof(zhOpts);
        return zhOpts;
    }
    n = _countof(enOpts);
    return enOpts;
}

inline const wchar_t* DisplayForValue(const SettingOption* opts, size_t n, const std::string& value) {
    for (size_t i = 0; i < n; ++i) {
        if (value == opts[i].value) return opts[i].display;
    }
    return (n > 0) ? opts[0].display : L"";
}

inline std::string ValueForDisplay(const SettingOption* opts, size_t n, const wchar_t* display) {
    for (size_t i = 0; i < n; ++i) {
        if (wcscmp(display, opts[i].display) == 0) return opts[i].value;
    }
    return (n > 0) ? std::string(opts[0].value) : std::string();
}
