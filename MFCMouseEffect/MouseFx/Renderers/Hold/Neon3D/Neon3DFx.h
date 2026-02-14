#pragma once

#include "Neon3DBezier.h"
#include "Neon3DRings.h"

#include <gdiplus.h>
#include <cstdint>
#include <vector>

namespace mousefx {

// Forward declaration from EffectComputeExecutor
namespace compute {
    enum class ParallelProfile;
}

namespace neon3d {

static const float kTau = 6.2831853f;
static const float kStartAng = -3.1415926f / 2.0f;

struct Spark {
    float ang = 0.0f;
    float rr = 0.0f;
    float v = 0.0f;
    float life = 0.0f;
};

void DrawInnerScanner(Gdiplus::Graphics& g, float cx, float cy, float r, float timeSec, float alpha01,
                      const Gdiplus::Color& cyan, const Gdiplus::Color& purple);

void DrawMicroStreaks(Gdiplus::Graphics& g, float cx, float cy, float r, float appear01, float timeSec, const Gdiplus::Color& cyan);

void DrawInnerHatch(Gdiplus::Graphics& g, float cx, float cy, float innerR, float progress01, float headAng, float timeSec,
                    const Gdiplus::Color& cyan);

void DrawCrystalSeed(Gdiplus::Graphics& g, float cx, float cy, float size, float timeSec, float appear01,
                     const Gdiplus::Color& cyan, const Gdiplus::Color& purple);

void UpdateSparks(std::vector<Spark>& sparks, float dtSec);

void DrawBranchTendrils(Gdiplus::Graphics& g, float cx, float cy, float innerR, float progress01, float headAng, float timeSec, float dtSec,
                        float alpha01, float bundleBiasRad,
                        const Gdiplus::Color& cyan, const Gdiplus::Color& purple, const float* seeds, int seedsCount,
                        int branchCount, std::vector<Spark>& sparks, LcgRng& rng);

void DrawWovenBand(Gdiplus::Graphics& g, float cx, float cy, float R, float progress01, float headAng, float timeSec,
                   const Gdiplus::Color& cyan, const Gdiplus::Color& purple, const Gdiplus::Color& mint);

void DrawPercentLabel(Gdiplus::Graphics& g, float cx, float cy, float R, float progress01, float headAng,
                      float alpha01, float outwardPx,
                      const Gdiplus::Color& cyan, const Gdiplus::Color& purple);

} // namespace neon3d
} // namespace mousefx
