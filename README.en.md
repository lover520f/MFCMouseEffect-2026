# MFCMouseEffect

<p align="center">
  <img src="./MFCMouseEffect/res/logo_elegant.png" width="128" alt="MFCMouseEffect Logo">
</p>

**[🇨🇳 中文](README.md)** | **🇬🇧 English**

---

**MFCMouseEffect** is a lightweight, high-performance Windows desktop enhancement tool designed to provide real-time visual feedback (ripples, particle trails, text effects, etc.) to elevate your interaction experience.

### 🌟 Key Features
- **Various Click Effects**: Support for ripple animations, random floating text, click bursts, and more.
- **Dynamic Mouse Trails**: Elegant particle flows following mouse movement with various color themes (e.g., Rainbow, Aurora).
- **Scroll & Hover Feedback**: Delicate visual guidance not just for clicks, but also for wheel scrolling and mouse hovering.
- **Extreme Performance**: Built with C++/MFC and utilizing GDI+ for hardware-accelerated rendering, ensuring low CPU and memory usage.
- **Process Singleton & Trayized**: Ensures a single instance automatically and supports running in the system tray.

### 📸 Showcase of Effects

| | |
| :---: | :---: |
| <img src="./docs/images/setting_en.png" width="350"><br>**Settings Interface** | <img src="./docs/images/ripple_concept.png" width="350"><br>**Click Ripple** |
| <img src="./docs/images/trail_concept.png" width="350"><br>**Particle Trail** | <img src="./docs/images/scroll_concept.png" width="350"><br>**Scroll Feedback** |
| <img src="./docs/images/hold_concept.png" width="350"><br>**Long Press (Hold)** | <img src="./docs/images/hover_concept.png" width="350"><br>**Hover Glow** |

### 🎨 Themes & Customization
You can easily switch between different visual themes (e.g., Rainbow, Aurora, Neon) in the settings window. Each effect can be independently toggled and configured to match your personal style.

---

## 🛠 Installation & Usage

### Build
1. Open `MFCMouseEffect.sln` with Visual Studio 2022.
2. Select `Release | x64` configuration.
3. Run `Build -> Rebuild Solution`.
4. Run `x64/Release/MFCMouseEffect.exe`.

### Installer
Use our [Inno Setup Script](./Install/MFCMouseEffect.iss) to build a professional installer.

---

## 📂 Project Structure
- **MFCMouseEffect/**: UI & App logic.
- **MouseFx/**: Core effect engine.
- **docs/**: Documentation ([UI Refinement](./docs/ui_refinement.md), [Implementation](./docs/singleton_implementation.md)).
- **Install/**: Inno Setup scripts.

---

## ⚖️ License
[MIT License](./LICENSE)

---
**⭐ If you find this useful, please give it a Star!**
