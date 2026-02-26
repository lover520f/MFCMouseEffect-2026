#include "pch.h"

#include "Platform/macos/Shell/MacosTrayMenuFactory.Internal.h"

namespace mousefx::macos_tray::menu_factory_detail {

NSMenuItem* CreateSettingsMenuItem(id actionBridge, NSString* settingsTitle) {
#if !defined(__APPLE__)
    (void)actionBridge;
    (void)settingsTitle;
    return nil;
#else
    NSMenuItem* settingsItem = [[NSMenuItem alloc] initWithTitle:settingsTitle
                                                           action:@selector(onOpenSettings:)
                                                    keyEquivalent:@","];
    if (settingsItem == nil) {
        return nil;
    }
    [settingsItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
    [settingsItem setTarget:actionBridge];
    return settingsItem;
#endif
}

NSMenuItem* CreateExitMenuItem(id actionBridge, NSString* exitTitle) {
#if !defined(__APPLE__)
    (void)actionBridge;
    (void)exitTitle;
    return nil;
#else
    NSMenuItem* exitItem = [[NSMenuItem alloc] initWithTitle:exitTitle
                                                       action:@selector(onExit:)
                                                keyEquivalent:@"q"];
    if (exitItem == nil) {
        return nil;
    }
    [exitItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
    [exitItem setTarget:actionBridge];
    return exitItem;
#endif
}

void ConfigureStatusButton(NSStatusItem* statusItem, NSString* tooltip) {
#if !defined(__APPLE__)
    (void)statusItem;
    (void)tooltip;
#else
    NSStatusBarButton* button = [statusItem button];
    if (button != nil) {
        [button setTitle:@"MFX"];
        [button setToolTip:tooltip];
    }
#endif
}

} // namespace mousefx::macos_tray::menu_factory_detail
