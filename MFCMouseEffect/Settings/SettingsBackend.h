#pragma once

#include <memory>
#include <string>

#include "SettingsModel.h"

namespace mousefx {
class AppController;
}

class ISettingsBackend {
public:
    virtual ~ISettingsBackend() = default;
    virtual SettingsModel Load() = 0;
    virtual void Apply(const SettingsModel& model) = 0;
};

std::unique_ptr<ISettingsBackend> CreateSettingsBackend(mousefx::AppController* controller);
