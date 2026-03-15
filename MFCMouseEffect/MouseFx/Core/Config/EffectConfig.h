#pragma once

#include "MouseFx/Styles/RippleStyle.h"  // For Argb
#include <string>
#include <vector>
#include <cstdint>
#include <map>

namespace mousefx {

// Parse ARGB from "#AARRGGBB" or "#RRGGBB" hex string
Argb ArgbFromHex(const std::string& hex);

// Configuration for Floating Text effect
struct TextConfig {
    int durationMs = 800;
    int floatDistance = 60;
    std::wstring fontFamily = L"Microsoft YaHei";
    float fontSize = 8.0f;
    
    // Random pool
    std::vector<std::wstring> texts = { 
        L"\u7f8e\u4e3d",
        L"\u5065\u5eb7",
        L"\u5e78\u798f",
        L"\u8d22\u5bcc+1w",
        L"\u5f00\u5fc3"
    };
    std::vector<Argb> colors = { 
        {0xFFFF69B4}, // HotPink
        {0xFF87CEEB}, // SkyBlue
        {0xFFFFD700}, // Gold
        {0xFF32CD32}, // LimeGreen
        {0xFFBA55D3}  // MediumOrchid
    };
};

// Configuration for Ripple effect
struct RippleConfig {
    int durationMs = 350;
    float startRadius = 0.0f;
    float endRadius = 40.0f;
    float strokeWidth = 2.5f;
    int windowSize = 120;
    
    // Per-button colors
    struct ButtonColors {
        Argb fill;
        Argb stroke;
        Argb glow;
    };
    
    ButtonColors leftClick{ {0x594FC3F7}, {0xFF0288D1}, {0x660288D1} };
    ButtonColors rightClick{ {0x50FFB74D}, {0xFFFF6F00}, {0x55FF6F00} };
    ButtonColors middleClick{ {0x5033D17A}, {0xFF0B8043}, {0x550B8043} };
};

// Configuration for Trail effect
struct TrailConfig {
    int durationMs = 350;
    int maxPoints = 20;
    float lineWidth = 4.0f;
    Argb color{ 0xDC64FFDA }; // Light cyan-green
};

// History profile for strategy-based trail renderers (line/streamer/electric/meteor/tubes).
// Note: ParticleTrailEffect uses a different window/pipeline.
struct TrailHistoryProfile {
    int durationMs = 300;
    int maxPoints = 32;
};

struct TrailProfilesConfig {
    TrailHistoryProfile line{300, 32};
    TrailHistoryProfile streamer{420, 46};
    TrailHistoryProfile electric{280, 24};
    TrailHistoryProfile meteor{520, 60};
    TrailHistoryProfile tubes{350, 40};
};

struct StreamerTrailParams {
    float glowWidthScale = 1.8f;
    float coreWidthScale = 0.55f;
    float headPower = 1.6f;
};

struct ElectricTrailParams {
    float amplitudeScale = 1.0f;
    float forkChance = 0.10f; // base chance per segment (multiplied by life)
};

struct MeteorTrailParams {
    float sparkRateScale = 1.0f;
    float sparkSpeedScale = 1.0f;
};

struct IdleFadeParams {
    // 0 means "use renderer default"
    int startMs = 0;
    int endMs = 0;
};

struct TrailRendererParamsConfig {
    StreamerTrailParams streamer;
    ElectricTrailParams electric;
    MeteorTrailParams meteor;
    IdleFadeParams idleFade;
};

// Configuration for Icon/Star effect
struct IconConfig {
    int durationMs = 350;
    float startRadius = 5.0f;
    float endRadius = 35.0f;
    float strokeWidth = 2.0f;
    Argb fillColor{ 0xFFFFD700 }; // Gold
    Argb strokeColor{ 0xFFFF8C00 }; // Dark Orange
};

struct PerMonitorPosOverride {
    bool enabled = false;
    int absoluteX = 40;
    int absoluteY = 40;
};

// Configuration for input action indicator (mouse/keyboard overlay).
struct InputIndicatorConfig {
    bool enabled = true;
    bool keyboardEnabled = true;
    // Renderer backend: "native" | "wasm".
    std::string renderMode = "native";
    // When renderMode == "wasm", fallback to native overlay if wasm does not render.
    bool wasmFallbackToNative = true;
    // Selected indicator plugin manifest path (UTF-8 path string).
    std::string wasmManifestPath;

    // --- Position settings (shared by mouse and keyboard) ---
    // Cross-platform contract uses top-left screen-space semantics:
    // +X points right, +Y points down.
    std::string positionMode = "relative"; // relative | absolute
    int offsetX = 24;
    int offsetY = 24;
    int absoluteX = 40;
    int absoluteY = 40;
    std::string targetMonitor = "cursor"; // cursor | primary | monitor_N | custom

    // --- Common visual ---
    int sizePx = 72;
    int durationMs = 420;

    // Display mode: "all", "significant", "shortcut"
    std::string keyDisplayMode = "all";
    // Key label layout mode:
    // - "fixed_font"  (keep font size and expand indicator width if needed)
    // - "fixed_area"  (keep indicator area and shrink font if needed)
    std::string keyLabelLayoutMode = "fixed_font";

    // Per-monitor overrides (monitorId -> {enabled, x, y})
    std::map<std::string, PerMonitorPosOverride> perMonitorOverrides;
};

struct AutomationKeyBinding {
    struct GesturePoint {
        int x = 0;
        int y = 0;
    };

    struct GesturePattern {
        std::string mode = "preset";
        int matchThresholdPercent = 75;
        std::vector<GesturePoint> customPoints;
        std::vector<std::vector<GesturePoint>> customStrokes;
    };

    struct ModifierCondition {
        // any  = ignore modifier state
        // none = require no primary/shift/alt modifier
        // exact = exact-match selected modifiers
        std::string mode = "any";
        bool primary = false;
        bool shift = false;
        bool alt = false;
    };

    bool enabled = true;
    std::string trigger;
    // left | middle | right | none (gesture-only, no button required)
    std::string triggerButton = "left";
    // Scope list format:
    // - "all"
    // - "process:<process_name>" (for example "process:code", "process:code.exe", "process:safari.app")
    //
    // Notes:
    // - "all" means globally active.
    // - process scopes are OR-matched (any listed app can trigger this binding).
    // - if "all" exists, process scopes are ignored.
    std::vector<std::string> appScopes = {"all"};
    GesturePattern gesturePattern{};
    ModifierCondition modifiers{};
    std::string keys;
};

struct GestureAutomationConfig {
    bool enabled = false;
    // left | middle | right | none (default button for new gesture mappings)
    std::string triggerButton = "left";
    int minStrokeDistancePx = 80;
    int sampleStepPx = 10;
    int maxDirections = 4;
    std::vector<AutomationKeyBinding> mappings;
};

struct InputAutomationConfig {
    bool enabled = false;
    std::vector<AutomationKeyBinding> mouseMappings;
    GestureAutomationConfig gesture;
};

struct WasmConfig {
    // Whether WASM host is enabled after startup.
    bool enabled = false;
    // Fallback to built-in click effect when WASM call does not render output.
    bool fallbackToBuiltinClick = true;
    // Selected plugin manifest path (UTF-8 path string).
    std::string manifestPath;
    // Optional per-category plugin manifest paths (UTF-8 path string).
    // Empty value falls back to `manifestPath`.
    std::string manifestPathClick;
    std::string manifestPathTrail;
    std::string manifestPathScroll;
    std::string manifestPathHold;
    std::string manifestPathHover;
    // Optional extra catalog root path (UTF-8 path string) for plugin discovery.
    std::string catalogRootPath;
    // WASM execution budget policy.
    uint32_t outputBufferBytes = 16u * 1024u;
    uint32_t maxCommands = 256u;
    double maxEventExecutionMs = 1.0;
};

struct EffectSizeScaleConfig {
    // Percentage scale per effect category (100 = default size).
    int click = 100;
    int trail = 100;
    int scroll = 100;
    int hold = 100;
    int hover = 100;
};

struct EffectConflictPolicyConfig {
    // hold_only | move_only | blend
    std::string holdMovePolicy = "hold_only";
};

// Active effect selections per category (persisted).
struct ActiveEffectConfig {
    std::string click = "text";
    std::string trail = "particle";
    std::string scroll = "arrow";
    std::string hover = "glow";
    std::string hold = "charge";
};

// Root configuration container
struct EffectConfig {
    std::string defaultEffect = "ripple";
    std::string theme = "neon";
    // Launch app automatically when user logs in.
    bool launchAtStartup = false;
    // Optional extra theme package root path (UTF-8 path string).
    std::string themeCatalogRootPath;
    // Overlay target FPS for effect windows.
    // 0 means auto (follow display max refresh).
    // Positive value means explicit cap.
    int overlayTargetFps = 0;
    // UI language for non-background settings window (persisted).
    // Values: "zh-CN" (default) or "en-US".
    std::string uiLanguage = "zh-CN";
    // Hold follow strategy:
    // precise | smooth | efficient
    std::string holdFollowMode = "smooth";
    // Hold presenter backend strategy:
    // auto | <backend_id>
    std::string holdPresenterBackend = "auto";
    // Process names where cursor effects should be disabled.
    // Entries accept process base names with/without suffix,
    // for example: "code", "code.exe", "safari.app".
    std::vector<std::string> effectsBlacklistApps{};
    ActiveEffectConfig active;
    
    RippleConfig ripple;
    TrailConfig trail;
    TrailProfilesConfig trailProfiles;
    std::string trailStyle = "default"; // default | snappy | long | cinematic | custom
    TrailRendererParamsConfig trailParams;
    IconConfig icon;
    TextConfig textClick;
    EffectSizeScaleConfig effectSizeScales;
    EffectConflictPolicyConfig effectConflictPolicy;
    InputIndicatorConfig inputIndicator;
    InputAutomationConfig automation;
    WasmConfig wasm;
    
    TrailHistoryProfile GetTrailHistoryProfile(const std::string& type) const;
    
    // Load config from file, merging with defaults.
    // If file doesn't exist or has errors, returns defaults (no crash).
    static EffectConfig Load(const std::wstring& exeDir);

    // Save config to file (best effort). Returns true if write succeeded.
    static bool Save(const std::wstring& exeDir, const EffectConfig& cfg);
    
    // Get built-in defaults
    static EffectConfig GetDefault();
};

} // namespace mousefx
