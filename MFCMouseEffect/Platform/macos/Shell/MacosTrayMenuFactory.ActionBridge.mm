#include "pch.h"

#include "Platform/macos/Shell/MacosTrayMenuFactory.Internal.h"

@interface MfxMacTrayActionBridge : NSObject {
@private
    mousefx::IAppShellHost* host_;
}

- (instancetype)initWithHost:(mousefx::IAppShellHost*)host;
- (void)onOpenSettings:(id)sender;
- (void)onExit:(id)sender;

@end

@implementation MfxMacTrayActionBridge

- (instancetype)initWithHost:(mousefx::IAppShellHost*)host {
    self = [super init];
    if (self != nil) {
        host_ = host;
    }
    return self;
}

- (void)onOpenSettings:(id)sender {
    (void)sender;
    if (host_ != nullptr) {
        host_->OpenSettingsFromShell();
    }
}

- (void)onExit:(id)sender {
    (void)sender;
    if (host_ != nullptr) {
        host_->RequestExitFromShell();
    }
}

@end

namespace mousefx::macos_tray::menu_factory_detail {

id CreateTrayActionBridge(IAppShellHost* host) {
#if !defined(__APPLE__)
    (void)host;
    return nil;
#else
    return [[MfxMacTrayActionBridge alloc] initWithHost:host];
#endif
}

} // namespace mousefx::macos_tray::menu_factory_detail
