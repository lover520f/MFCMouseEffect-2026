#include "pch.h"
#include "QuantumHaloPresenterBackendRegistry.h"

#include <algorithm>

namespace mousefx {

QuantumHaloPresenterBackendRegistry& QuantumHaloPresenterBackendRegistry::Instance() {
    static QuantumHaloPresenterBackendRegistry instance;
    return instance;
}

void QuantumHaloPresenterBackendRegistry::Register(
    const std::string& name,
    int priority,
    Factory factory) {
    if (name.empty() || !factory) {
        return;
    }

    auto it = entries_.find(name);
    if (it == entries_.end()) {
        Entry entry{};
        entry.priority = priority;
        entry.order = nextOrder_++;
        entry.factory = std::move(factory);
        entries_.emplace(name, std::move(entry));
        return;
    }

    it->second.priority = priority;
    it->second.factory = std::move(factory);
}

std::unique_ptr<IQuantumHaloPresenterBackend> QuantumHaloPresenterBackendRegistry::Create(
    const std::string& name) const {
    const auto it = entries_.find(name);
    if (it == entries_.end()) {
        return nullptr;
    }
    if (!it->second.factory) {
        return nullptr;
    }
    return it->second.factory();
}

std::vector<QuantumHaloPresenterBackendRegistry::Descriptor> QuantumHaloPresenterBackendRegistry::ListByPriority() const {
    struct RankedDescriptor {
        Descriptor desc{};
        uint64_t order = 0;
    };

    std::vector<RankedDescriptor> ranked;
    ranked.reserve(entries_.size());

    for (const auto& kv : entries_) {
        RankedDescriptor item{};
        item.desc.name = kv.first;
        item.desc.priority = kv.second.priority;
        item.order = kv.second.order;
        ranked.push_back(std::move(item));
    }

    std::sort(ranked.begin(), ranked.end(), [](const RankedDescriptor& a, const RankedDescriptor& b) {
        if (a.desc.priority != b.desc.priority) {
            return a.desc.priority > b.desc.priority;
        }
        return a.order < b.order;
    });

    std::vector<Descriptor> output;
    output.reserve(ranked.size());
    for (const auto& item : ranked) {
        output.push_back(item.desc);
    }
    return output;
}

} // namespace mousefx
