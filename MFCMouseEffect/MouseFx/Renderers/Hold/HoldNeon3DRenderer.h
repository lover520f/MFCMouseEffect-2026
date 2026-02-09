#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"
#include "Neon3D/Neon3DFx.h"
#include "Neon3D/Neon3DColor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace mousefx {

// Neon HUD-inspired hold indicator (glass ring + woven band + crystal seed + tendrils).
// Ported from the HTML "Hold Concept Demo v2" reference (layer-by-layer).
class HoldNeon3DRenderer final : public IRippleRenderer {
public:
    void Start(const RippleStyle& style) override {
        currentHoldMs_ = 0;
        thresholdMs_ = style.durationMs;
        lastElapsedMs_ = 0;
        holdBiasMs_ = 0;
        holdBiasValid_ = false;
        sparks_.clear();
        sparks_.reserve(96);

        seed_ = (uint32_t)((uintptr_t)this ^ (uintptr_t)&seed_);
        static uint32_t s_nonce = 0x9E3779B9u;
        s_nonce = s_nonce * 1664525u + 1013904223u;
        seed_ ^= s_nonce;

        neon3d::StableHash hash(seed_);
        for (size_t i = 0; i < seeds_.size(); ++i) {
            seeds_[i] = hash.Hash01((uint32_t)(0xC001D00Du + i)) * 10.0f;
        }
        branchCount_ = 6 + (int)std::floor(hash.Hash01(0xB16B00u) * 4.0f); // 6..9
        bundleBiasRad_ = (hash.Hash01(0xA11CEu) - 0.5f) * 0.38f;           // small random offset
        orbitPhase_ = hash.Hash01(0xC0FFEEu) * neon3d::kTau;

        rng_ = neon3d::LcgRng(seed_ ^ 0xA5A5A5A5u);
    }

    void OnCommand(const std::string& cmd, const std::string& args) override {
        if (cmd == "hold_ms") {
            uint32_t ms = 0;
            if (sscanf_s(args.c_str(), "%u", &ms) == 1) SetHoldElapsedMs(ms);
            return;
        }
        if (cmd == "threshold_ms") {
            uint32_t ms = 0;
            if (sscanf_s(args.c_str(), "%u", &ms) == 1) SetHoldDurationMs(ms);
            return;
        }
    }

    void SetHoldElapsedMs(uint32_t holdMs) override {
        currentHoldMs_ = holdMs;
    }

    void SetHoldDurationMs(uint32_t thresholdMs) override {
        thresholdMs_ = thresholdMs;
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        using namespace neon3d;

        const float tn = Clamp01(t);
        const float timeSec = (float)elapsedMs / 1000.0f;

        float dtSec = 0.0f;
        if (elapsedMs > lastElapsedMs_) dtSec = (float)(elapsedMs - lastElapsedMs_) / 1000.0f;
        lastElapsedMs_ = elapsedMs;
        if (dtSec > 0.05f) dtSec = 0.05f;

        const float appearT = Clamp01((float)elapsedMs / 120.0f);
        const float alphaIn = Smoothstep(0.0f, 1.0f, appearT);
        const float scaleIn = 0.90f + 0.10f * EaseOutBack(appearT);

        const float cx = sizePx * 0.5f;
        const float cy = sizePx * 0.5f;
        const float baseR = std::min((float)style.endRadius + 10.0f, (float)sizePx * 0.42f);
        const float R = baseR * scaleIn;
        const float thick = std::max(12.0f, R * 0.18f);
        const float innerR = R - thick * 0.45f;

        const float progress = ComputeProgress(tn, elapsedMs, style.durationMs);
        // Concept alignment: progress tail anchors at 12 o'clock.
        const float progressStartAng = kStartAng;
        const float headAng = progressStartAng + progress * kTau;

        // Reduce early-frame clutter: fade in sub-layers progressively.
        const float hatchA = alphaIn * Smoothstep(0.03f, 0.16f, progress);
        const float tendrilsA = alphaIn * 0.86f * Smoothstep(0.08f, 0.26f, progress);
        const float weaveA = alphaIn * Smoothstep(0.10f, 0.30f, progress);
        const float capsuleA = alphaIn * Smoothstep(0.12f, 0.28f, progress);
        const float labelA = alphaIn * Smoothstep(0.08f, 0.22f, progress);

        const Gdiplus::Color strokeRaw = ToGdiPlus(style.stroke);
        const Gdiplus::Color primary(255, strokeRaw.GetR(), strokeRaw.GetG(), strokeRaw.GetB());

        // Palette: keep the concept's cyan/purple interplay even for chromatic/random themes.
        const NeonPalette pal = MakeNeonPalette(primary);
        const Gdiplus::Color cyan = pal.cyan;
        const Gdiplus::Color purple = pal.purple;
        const Gdiplus::Color mint = pal.mint;

        const Gdiplus::Color cyanHatch = MulAlpha(cyan, hatchA);
        const Gdiplus::Color cyanWeave = MulAlpha(cyan, weaveA);
        const Gdiplus::Color purpleWeave = MulAlpha(purple, weaveA);
        const Gdiplus::Color mintWeave = MulAlpha(mint, weaveA);

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);

        // Layer order matches the concept spec (bottom -> top).
        DrawGlassRing(g, cx, cy, R, thick, cyan, purple, alphaIn);
        // Micro streaks keep 12 o'clock as the "scan" origin (background texture).
        DrawMicroStreaks(g, cx, cy, innerR, alphaIn * 0.90f, timeSec, cyan);
        DrawInnerHatch(g, cx, cy, innerR, progress, headAng, timeSec, cyanHatch);
        // Keep scanner above hatch so the clockwise flow line remains readable.
        DrawInnerScanner(g, cx, cy, innerR - 2.0f, timeSec, alphaIn * 0.96f, cyan, purple);
        DrawCrystalSeed(g, cx, cy, 16.0f, timeSec, alphaIn, cyan, purple);

        // Keep the tendrils "alive" even when progress is clamped to 1.0.
        const float orbit = (0.20f + 0.16f * progress) * (float)sin(timeSec * 0.85f + orbitPhase_)
            + 0.05f * (float)sin(timeSec * 1.45f + orbitPhase_ * 2.3f);
        const float tendrilAng = headAng - 0.95f + orbit;
        DrawBranchTendrils(g, cx, cy, innerR, progress, tendrilAng, timeSec, dtSec, tendrilsA, bundleBiasRad_,
            cyan, purple, seeds_.data(), (int)seeds_.size(), branchCount_, sparks_, rng_);

        DrawProgressArcBase(g, cx, cy, R, progressStartAng, progress, cyan, alphaIn);
        DrawProgressMainArc(g, cx, cy, R, progressStartAng, progress, cyan, purple, alphaIn);
        DrawStartAnchorPlate(g, cx, cy, R, progressStartAng, cyan, purple, alphaIn * 0.85f);
        DrawWovenBand(g, cx, cy, R, progress, headAng, timeSec, cyanWeave, purpleWeave, mintWeave);
        DrawCapsuleHead(g, cx, cy, R, progress, headAng, cyan, purple, capsuleA);

        const float maxOut = std::max(6.0f, (float)sizePx * 0.5f - R - 18.0f);
        const float labelOut = std::min(22.0f, maxOut);
        DrawPercentLabel(g, cx, cy, R, progress, headAng, labelA, labelOut, cyan, purple);
    }

private:
    float ComputeProgress(float t01, uint64_t elapsedMs, uint32_t defaultThresholdMs) {
        using namespace render_utils;
        const uint32_t threshold = thresholdMs_ ? thresholdMs_ : defaultThresholdMs;
        if (threshold == 0) return Clamp01(t01);

        // If an upstream hold_ms arrives, align (once) the renderer's local elapsedMs to it,
        // so progress keeps increasing smoothly even if hold_ms updates are sparse.
        if (currentHoldMs_ > 0 && !holdBiasValid_) {
            holdBiasMs_ = (int64_t)currentHoldMs_ - (int64_t)elapsedMs;
            holdBiasValid_ = true;
        }

        int64_t effectiveMs = (int64_t)elapsedMs;
        if (holdBiasValid_) effectiveMs += holdBiasMs_;
        if (effectiveMs < 0) effectiveMs = 0;
        return Clamp01((float)effectiveMs / (float)threshold);
    }

    uint32_t currentHoldMs_ = 0;
    uint32_t thresholdMs_ = 0;
    uint32_t seed_ = 0;
    neon3d::LcgRng rng_{1u};
    std::array<float, 12> seeds_{};
    std::vector<neon3d::Spark> sparks_;
    int branchCount_ = 8;
    float bundleBiasRad_ = 0.0f;
    float orbitPhase_ = 0.0f;
    uint64_t lastElapsedMs_ = 0;
    int64_t holdBiasMs_ = 0;
    bool holdBiasValid_ = false;
};

REGISTER_RENDERER("hold_neon3d", HoldNeon3DRenderer)
static mousefx::RendererRegistrar<HoldNeon3DRenderer> reg_HoldNeon3DRenderer_neon3d("neon3d");

} // namespace mousefx
