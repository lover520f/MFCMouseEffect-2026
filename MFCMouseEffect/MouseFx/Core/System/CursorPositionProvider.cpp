#include "pch.h"

#include "MouseFx/Core/System/CursorPositionProvider.h"

#include "MouseFx/Core/System/NullCursorPositionService.h"
#include "Platform/PlatformInputServicesFactory.h"

#include <memory>

namespace mousefx {
namespace {

ICursorPositionService& CursorPositionServiceInstance() {
    static std::unique_ptr<ICursorPositionService> service =
        platform::CreateCursorPositionService();
    static NullCursorPositionService fallbackService{};
    if (service) {
        return *service;
    }
    return fallbackService;
}

} // namespace

bool TryGetCursorScreenPoint(ScreenPoint* outPt) {
    return CursorPositionServiceInstance().TryGetCursorScreenPoint(outPt);
}

} // namespace mousefx
