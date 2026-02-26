#include "pch.h"

#include "Platform/macos/Shell/MacosTrayMenuFactory.Internal.h"
#include "Platform/macos/Shell/MacosTrayMenuFactory.h"

#include "Platform/macos/Shell/MacosTrayRuntimeHelpers.h"

namespace mousefx::macos_tray {

#if defined(__APPLE__)
bool BuildMacosTrayMenu(
    IAppShellHost* host,
    const MacosTrayMenuText& menuText,
    MacosTrayMenuObjects* outObjects) {
    if (host == nullptr || outObjects == nullptr) {
        return false;
    }

    NSString* settingsTitle = NsStringFromUtf8OrDefault(menuText.settingsTitle, @"Settings");
    NSString* exitTitle = NsStringFromUtf8OrDefault(menuText.exitTitle, @"Exit");
    NSString* tooltip = NsStringFromUtf8OrDefault(menuText.tooltip, @"MFCMouseEffect");

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

    NSStatusItem* statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];
    id actionBridge = menu_factory_detail::CreateTrayActionBridge(host);
    NSMenu* menu = [[NSMenu alloc] initWithTitle:@"MFCMouseEffect"];
    if (statusItem == nil || actionBridge == nil || menu == nil) {
        if (menu != nil) {
            [menu release];
        }
        if (actionBridge != nil) {
            [actionBridge release];
        }
        if (statusItem != nil) {
            [[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
            [statusItem release];
        }
        return false;
    }

    NSMenuItem* settingsItem = menu_factory_detail::CreateSettingsMenuItem(actionBridge, settingsTitle);
    if (settingsItem == nil) {
        [menu release];
        [actionBridge release];
        [[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
        [statusItem release];
        return false;
    }
    [menu addItem:settingsItem];
    [settingsItem release];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem* exitItem = menu_factory_detail::CreateExitMenuItem(actionBridge, exitTitle);
    if (exitItem == nil) {
        [menu release];
        [actionBridge release];
        [[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
        [statusItem release];
        return false;
    }
    [menu addItem:exitItem];
    [exitItem release];

    [statusItem setMenu:menu];
    menu_factory_detail::ConfigureStatusButton(statusItem, tooltip);

    outObjects->statusItem = statusItem;
    outObjects->menu = menu;
    outObjects->actionBridge = actionBridge;
    return true;
}

void ReleaseMacosTrayMenu(MacosTrayMenuObjects* objects) {
    if (objects == nullptr) {
        return;
    }

    if (objects->statusItem != nil) {
        [[NSStatusBar systemStatusBar] removeStatusItem:objects->statusItem];
        [objects->statusItem release];
        objects->statusItem = nil;
    }
    if (objects->menu != nil) {
        [objects->menu release];
        objects->menu = nil;
    }
    if (objects->actionBridge != nil) {
        [objects->actionBridge release];
        objects->actionBridge = nil;
    }
}
#endif

} // namespace mousefx::macos_tray
