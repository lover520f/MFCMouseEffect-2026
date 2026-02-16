#include "pch.h"
#include "KeyboardInjector.h"

#include "KeyChord.h"

#include <vector>

namespace mousefx {
namespace {

constexpr ULONG_PTR kInjectedExtraInfo = 0x4D46584B; // "MFXK"

INPUT MakeKeyInput(UINT vk, bool keyUp) {
    INPUT input{};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(vk);
    input.ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    input.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : 0;
    input.ki.dwExtraInfo = kInjectedExtraInfo;
    return input;
}

} // namespace

bool KeyboardInjector::SendChord(const std::string& chordText) const {
    KeyChord chord{};
    if (!ParseKeyChord(chordText, &chord)) {
        return false;
    }

    std::vector<INPUT> inputs;
    inputs.reserve(chord.modifiers.size() * 2 + 2);

    for (UINT vk : chord.modifiers) {
        inputs.push_back(MakeKeyInput(vk, false));
    }

    inputs.push_back(MakeKeyInput(chord.key, false));
    inputs.push_back(MakeKeyInput(chord.key, true));

    for (auto it = chord.modifiers.rbegin(); it != chord.modifiers.rend(); ++it) {
        inputs.push_back(MakeKeyInput(*it, true));
    }

    if (inputs.empty()) {
        return false;
    }

    const UINT sent = SendInput(
        static_cast<UINT>(inputs.size()),
        inputs.data(),
        static_cast<int>(sizeof(INPUT)));
    return sent == inputs.size();
}

} // namespace mousefx
