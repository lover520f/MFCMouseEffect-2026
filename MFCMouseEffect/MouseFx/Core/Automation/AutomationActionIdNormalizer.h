#pragma once

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <string>

namespace mousefx::automation_ids {

inline std::string NormalizeTextId(std::string value) {
    value = ToLowerAscii(TrimAscii(std::move(value)));
    std::replace(value.begin(), value.end(), '-', '_');
    std::replace(value.begin(), value.end(), ' ', '_');
    return value;
}

inline std::string NormalizeMouseActionId(std::string value) {
    value = NormalizeTextId(std::move(value));
    if (value == "left" || value == "leftclick" || value == "lclick") {
        return "left_click";
    }
    if (value == "right" || value == "rightclick" || value == "rclick") {
        return "right_click";
    }
    if (value == "middle" || value == "middleclick" || value == "mclick") {
        return "middle_click";
    }
    if (value == "wheel_up" || value == "scrollup") {
        return "scroll_up";
    }
    if (value == "wheel_down" || value == "scrolldown") {
        return "scroll_down";
    }
    return value;
}

inline std::string NormalizeGestureId(std::string value) {
    value = NormalizeTextId(std::move(value));
    if (value == "line_right" || value == "line" || value == "hline") {
        return "right";
    }
    if (value == "line_left") {
        return "left";
    }
    if (value == "line_down" || value == "vline") {
        return "down";
    }
    if (value == "line_up") {
        return "up";
    }
    if (value == "slash") {
        return "diag_down_right";
    }
    if (value == "backslash") {
        return "diag_down_left";
    }
    if (value == "v") {
        return "diag_down_right_diag_up_right";
    }
    if (value == "caret" || value == "inverted_v") {
        return "diag_up_right_diag_down_right";
    }
    if (value == "w") {
        return "diag_down_right_diag_up_right_diag_down_right";
    }
    if (value == "diag_down_right_diag_up_right_diag_down_right_diag_up_right") {
        return "diag_down_right_diag_up_right_diag_down_right";
    }
    return value;
}

} // namespace mousefx::automation_ids
