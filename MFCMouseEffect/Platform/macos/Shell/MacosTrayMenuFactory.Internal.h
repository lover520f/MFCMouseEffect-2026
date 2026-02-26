#pragma once

#include "MouseFx/Core/Shell/IAppShellHost.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_tray::menu_factory_detail {

#if defined(__APPLE__)
id CreateTrayActionBridge(IAppShellHost* host);
NSMenuItem* CreateSettingsMenuItem(id actionBridge, NSString* settingsTitle);
NSMenuItem* CreateExitMenuItem(id actionBridge, NSString* exitTitle);
void ConfigureStatusButton(NSStatusItem* statusItem, NSString* tooltip);
#endif

} // namespace mousefx::macos_tray::menu_factory_detail
