#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include "MouseFx/Core/GlobalMouseHook.h"

namespace mousefx {

// Effect categories based on trigger type.
// Each category can have at most one active effect.
// Multiple categories can be active simultaneously.
enum class EffectCategory : uint8_t {
    Trail,   // Triggered by mouse movement
    Click,   // Triggered by mouse button press/release
    Hover,   // Triggered when mouse is idle for N seconds
    Scroll,  // Triggered by mouse wheel
    Edge,    // Triggered when mouse hits screen edge
    Hold,    // Triggered when mouse button is held down

    Count    // Number of categories (for array sizing)
};

// Scroll event data
struct ScrollEvent {
    POINT pt;       // Mouse position
    int delta;      // Scroll amount (positive = up, negative = down)
    bool horizontal;// True if horizontal scroll
};

// Edge event data
struct EdgeEvent {
    POINT pt;       // Mouse position
    enum Edge { Left = 1, Right = 2, Top = 4, Bottom = 8 } edge;
};

// Interface for all mouse effects.
class IMouseEffect {
public:
    virtual ~IMouseEffect() = default;

    // Returns the category this effect belongs to.
    virtual EffectCategory Category() const = 0;

    // Returns the effect type name (e.g., "ripple", "line_trail").
    virtual const char* TypeName() const = 0;

    // Called when the effect should initialize.
    virtual bool Initialize() = 0;

    // Called when the effect should cleanup resources.
    virtual void Shutdown() = 0;

    // Category-specific event handlers.
    // Override only the ones relevant to your effect's category.
    virtual void OnClick(const ClickEvent& event) { (void)event; }
    virtual void OnMouseMove(const POINT& pt) { (void)pt; }
    virtual void OnScroll(const ScrollEvent& event) { (void)event; }
    virtual void OnHoverStart(const POINT& pt) { (void)pt; }
    virtual void OnHoverEnd() {}
    virtual void OnHoldStart(const POINT& pt, int button) { (void)pt; (void)button; }
    virtual void OnHoldUpdate(const POINT& pt, DWORD durationMs) { (void)pt; (void)durationMs; }
    virtual void OnHoldEnd() {}
    virtual void OnEdgeHit(const EdgeEvent& event) { (void)event; }
    
    // Interaction interface
    virtual void OnCommand(const std::string& cmd, const std::string& args) {}
};

// Helper to get category name string
inline const char* CategoryToString(EffectCategory cat) {
    switch (cat) {
        case EffectCategory::Trail:  return "trail";
        case EffectCategory::Click:  return "click";
        case EffectCategory::Hover:  return "hover";
        case EffectCategory::Scroll: return "scroll";
        case EffectCategory::Edge:   return "edge";
        case EffectCategory::Hold:   return "hold";
        default: return "unknown";
    }
}

// Helper to parse category from string
inline EffectCategory CategoryFromString(const std::string& str) {
    if (str == "trail")  return EffectCategory::Trail;
    if (str == "click")  return EffectCategory::Click;
    if (str == "hover")  return EffectCategory::Hover;
    if (str == "scroll") return EffectCategory::Scroll;
    if (str == "edge")   return EffectCategory::Edge;
    if (str == "hold")   return EffectCategory::Hold;
    return EffectCategory::Click; // Default
}

} // namespace mousefx
