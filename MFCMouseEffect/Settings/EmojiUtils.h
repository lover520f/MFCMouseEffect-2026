#pragma once

#include <cstdint>
#include <string>

namespace settings {

// Reads the next Unicode code point from UTF-16 text (handles surrogate pairs).
// Advances *i to the next position. Returns 0 on end-of-string.
uint32_t NextCodePointUtf16(const std::wstring& text, size_t* i);

bool IsEmojiCodePoint(uint32_t cp);
bool IsEmojiComponent(uint32_t cp);

} // namespace settings

