#pragma once

#include "IQuantumHaloPresenterBackend.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mousefx {

class QuantumHaloPresenterBackendRegistry {
public:
    using Factory = std::function<std::unique_ptr<IQuantumHaloPresenterBackend>()>;

    struct Descriptor {
        std::string name{};
        int priority = 0;
    };

    static QuantumHaloPresenterBackendRegistry& Instance();

    void Register(const std::string& name, int priority, Factory factory);
    std::unique_ptr<IQuantumHaloPresenterBackend> Create(const std::string& name) const;
    std::vector<Descriptor> ListByPriority() const;

private:
    struct Entry {
        int priority = 0;
        uint64_t order = 0;
        Factory factory{};
    };

    std::map<std::string, Entry> entries_{};
    uint64_t nextOrder_ = 0;
};

template <typename T>
class QuantumHaloPresenterBackendRegistrar {
public:
    QuantumHaloPresenterBackendRegistrar(const std::string& name, int priority) {
        QuantumHaloPresenterBackendRegistry::Instance().Register(name, priority, []() {
            return std::make_unique<T>();
        });
    }
};

#define REGISTER_QUANTUM_HALO_PRESENTER_BACKEND(Name, Priority, Class) \
    static mousefx::QuantumHaloPresenterBackendRegistrar<Class> reg_##Class(Name, Priority);

} // namespace mousefx
