# MFCMouseEffect

<p align="center">
  <img src="./MFCMouseEffect/res/logo_elegant.png" width="128" alt="MFCMouseEffect Logo">
</p>

<p align="center">
  <a href="../../releases/latest"><img src="https://img.shields.io/badge/release-latest-blue" alt="release"></a>
  <img src="https://img.shields.io/badge/status-beta-green" alt="status">
  <img src="https://img.shields.io/badge/license-MIT-brightgreen" alt="license">
  <img src="https://img.shields.io/badge/platform-Windows%2010%2B-lightgrey" alt="platform">
</p>

**[🇨🇳 中文](README.md)** | **🇬🇧 English**

---

A lightweight, high-performance Windows desktop effect tool that adds real-time visual feedback (ripples, particle trails, text effects, etc.) to mouse clicks, trails, wheel, hold, and hover.

## ✨ Highlights
- Global `WH_MOUSE_LL` hook + GDI+ layered windows; smooth, low CPU/memory.
- Themes (neon / minimal / game feel) and per-effect toggles with persistence.
- Tray mode for day-to-day use; background mode driven by parent process via stdin; `config.json` stored beside the exe.
- Settings (browser UI): opened from tray; served by an embedded loopback server; bilingual and supports advanced tuning (saved to `config.json`).

## 📸 Showcase
| | |
| :---: | :---: |
| <img src="./docs/images/setting_en.png" width="340"><br>Settings window (legacy) | <img src="./docs/images/ripple_concept.png" width="340"><br>Click ripple |
| <img src="./docs/images/trail_concept.png" width="340"><br>Particle trail | <img src="./docs/images/scroll_concept.png" width="340"><br>Scroll indicator |
| <img src="./docs/images/hold_concept.png" width="340"><br>Hold charge | <img src="./docs/images/hover_concept.png" width="340"><br>Hover glow |

## 🆕 Recent fixes/improvements
- Settings window now centers by default, drags without flicker; reverted to native control styling (no extra background fill).
- Virtual/tablet secondary display offset: coordinate normalization fallback is enabled (Jan 2026), fixing most virtual display offsets. See `docs/issues/virtual-display-coordinates.md`.

## ⬇️ Download
- Latest release: [Releases](../../releases/latest)
- All releases: [All releases](../../releases)

## 📦 Build & Run
1. Open `MFCMouseEffect.sln` with Visual Studio 2022.
2. Select `Release | x64`, then “Rebuild Solution”.
3. Run `x64/Release/MFCMouseEffect.exe`. Tray mode offers an exit menu; in non-background mode the tray “Settings...” opens the browser settings page.

## 🖥️ Usage
- Language/theme/effects: configure in the browser settings page; saved to `config.json`.
- Admin windows: run the app as Administrator if you need effects inside elevated apps.
- Background mode: no tray/UI, fully controlled via stdin JSON by the parent process.

## 📑 Docs & media
- Full docs: `./docs/README.md`
- Virtual display offset notes: `./docs/issues/virtual-display-coordinates.md`
- Screenshots: `./docs/images/setting_en.png`, ripple/trail concept shots in `./docs/images/`.

## 🧭 Repo hygiene & community (suggested)
- Fill repository About: Description + Topics (e.g., mouse-effect, ripple, tray, mfc, windows, overlay).
- Add Social Preview and keep badges/screenshots visible in the README.
- Enable Discussions and seed 3–5 “good first issues”:
  - Make the normalization threshold configurable and add debug logging.
  - Add a sci‑fi neon theme preset.
  - Provide a lightweight portable ZIP build alongside the installer.
  - Benchmark CPU usage across DPI/refresh scenarios.
  - Expand Troubleshooting/FAQ (EN).

## ⚖️ License
[MIT License](./LICENSE)

---
**If it helps, please star ⭐ and share feedback in Issues/Discussions.**
