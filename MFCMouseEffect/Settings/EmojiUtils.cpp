#include "pch.h"

#include "EmojiUtils.h"

namespace settings {

uint32_t NextCodePointUtf16(const std::wstring& text, size_t* i) {
    if (!i || *i >= text.size()) return 0;
    const wchar_t lead = text[*i];
    (*i)++;
    if (lead >= 0xD800 && lead <= 0xDBFF) {
        if (*i < text.size()) {
            const wchar_t trail = text[*i];
            if (trail >= 0xDC00 && trail <= 0xDFFF) {
                (*i)++;
                return (((uint32_t)lead - 0xD800) << 10) + ((uint32_t)trail - 0xDC00) + 0x10000;
            }
        }
    }
    return (uint32_t)lead;
}

bool IsEmojiCodePoint(uint32_t cp) {
    if (cp >= 0x1F300 && cp <= 0x1F5FF) return true;
    if (cp >= 0x1F600 && cp <= 0x1F64F) return true;
    if (cp >= 0x1F680 && cp <= 0x1F6FF) return true;
    if (cp >= 0x1F700 && cp <= 0x1F77F) return true;
    if (cp >= 0x1F900 && cp <= 0x1F9FF) return true;
    if (cp >= 0x1FA70 && cp <= 0x1FAFF) return true;
    if (cp >= 0x2600 && cp <= 0x27BF) return true;
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true;
    return false;
}

bool IsEmojiComponent(uint32_t cp) {
    if (IsEmojiCodePoint(cp)) return true;
    if (cp == 0xFE0F || cp == 0xFE0E || cp == 0x200D) return true;
    if (cp >= 0x1F3FB && cp <= 0x1F3FF) return true;
    return false;
}

bool HasEmojiStarter(const std::wstring& text) {
    for (size_t i = 0; i < text.size();) {
        const uint32_t cp = NextCodePointUtf16(text, &i);
        if (cp == 0) break;
        if (IsEmojiCodePoint(cp)) return true;
    }
    return false;
}

bool IsEmojiOnlyText(const std::wstring& text) {
    bool hasEmoji = false;
    for (size_t i = 0; i < text.size();) {
        uint32_t cp = NextCodePointUtf16(text, &i);
        if (cp == 0) break;
        if (cp == 0xFE0F || cp == 0xFE0E || cp == 0x200D) continue; // VS16/VS15/ZWJ
        if (cp >= 0x1F3FB && cp <= 0x1F3FF) continue; // skin tone modifiers
        if (IsEmojiCodePoint(cp)) {
            hasEmoji = true;
            continue;
        }
        return false;
    }
    return hasEmoji;
}

} // namespace settings
