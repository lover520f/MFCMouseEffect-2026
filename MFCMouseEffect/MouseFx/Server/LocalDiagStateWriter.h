#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace mousefx {

class LocalDiagStateWriter final {
public:
    LocalDiagStateWriter() = default;
    LocalDiagStateWriter(const LocalDiagStateWriter&) = delete;
    LocalDiagStateWriter& operator=(const LocalDiagStateWriter&) = delete;

    void TryWriteSnapshot(const std::wstring& exeDir, const std::string& stateJson, uint64_t nowMs) noexcept;

private:
    std::atomic<uint64_t> lastDumpMs_{0};
};

} // namespace mousefx

