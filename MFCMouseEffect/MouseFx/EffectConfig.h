#pragma once

#include "RippleStyle.h"  // For Argb
#include <string>
#include <vector>
#include <cstdint>

namespace mousefx {

// Parse ARGB from "#AARRGGBB" or "#RRGGBB" hex string
Argb ArgbFromHex(const std::string& hex);

// Configuration for Floating Text effect
struct TextConfig {
    int durationMs = 800;
    int floatDistance = 60;
    std::wstring fontFamily = L"Microsoft YaHei";
    float fontSize = 18.0f;
    
    // Random pool
    std::vector<std::wstring> texts = { L"美丽", L"健康", L"幸福", L"民主", L"文明", L"和谐" };
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

// Configuration for Icon/Star effect
struct IconConfig {
    int durationMs = 350;
    float startRadius = 5.0f;
    float endRadius = 35.0f;
    float strokeWidth = 2.0f;
    Argb fillColor{ 0xFFFFD700 }; // Gold
    Argb strokeColor{ 0xFFFF8C00 }; // Dark Orange
};

// Root configuration container
struct EffectConfig {
    std::string defaultEffect = "ripple";
    
    RippleConfig ripple;
    TrailConfig trail;
    IconConfig icon;
    TextConfig textClick;
    
    // Load config from file, merging with defaults.
    // If file doesn't exist or has errors, returns defaults (no crash).
    static EffectConfig Load(const std::wstring& exeDir);
    
    // Get built-in defaults
    static EffectConfig GetDefault();
};

} // namespace mousefx
