#pragma once

#include <windows.h>

namespace mousefx {

// Command IDs used by the tray popup menu and other UI components.
enum TrayMenuCmd : UINT {
    kCmdTrayExit = 1001,
    kCmdTraySettings = 1002,
    kCmdStarRepo = 1003,
    kCmdTrayReloadConfig = 1004,

    // Click category
    kCmdClickRipple = 2001,
    kCmdClickStar = 2002,
    kCmdClickText = 2003,
    kCmdClickNone = 2004,

    // Trail category
    kCmdTrailLine = 3001,
    kCmdTrailParticle = 3002,
    kCmdTrailNone = 3003,
    kCmdTrailStreamer = 3004,
    kCmdTrailElectric = 3005,
    kCmdTrailTubes = 3006,
    kCmdTrailMeteor = 3007,

    // Hover category
    kCmdHoverGlow = 4001,
    kCmdHoverNone = 4002,
    kCmdHoverTubes = 4003,

    // Scroll category
    kCmdScrollArrow = 5001,
    kCmdScrollNone = 5002,
    kCmdScrollHelix = 5003,

    // Edge category
    kCmdEdgeNone = 6001,

    // Hold category
    kCmdHoldCharge = 7001,
    kCmdHoldLightning = 7002,
    kCmdHoldHex = 7003,
    kCmdHoldNone = 7004,
    kCmdHoldHologram = 7005,
    kCmdHoldSciFi3D = 7005, // Alias
    kCmdHoldTechRing = 7006,
    kCmdHoldNeon3D = 7007,

    // Theme
    kCmdThemeChromatic = 8000,
    kCmdThemeSciFi = 8001,
    kCmdThemeNeon = 8002,
    kCmdThemeMinimal = 8003,
    kCmdThemeGame = 8004,
};

} // namespace mousefx
