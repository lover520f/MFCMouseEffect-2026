#pragma once

#include <cstdint>
#include <string>

namespace mousefx {

struct ScreenPoint {
    int32_t x = 0;
    int32_t y = 0;
};

enum class MouseButton : uint8_t {
    Left = 1,
    Right = 2,
    Middle = 3,
};

struct ClickEvent {
    ScreenPoint pt{};
    MouseButton button{MouseButton::Left};
};

struct KeyEvent {
    ScreenPoint pt{};
    uint32_t vkCode{0};
    bool systemKey{false};
    bool ctrl{false};
    bool shift{false};
    bool alt{false};
    bool win{false};
    std::wstring text{};
};

} // namespace mousefx
