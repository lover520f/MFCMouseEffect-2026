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

    bool ShouldWriteSnapshot(uint64_t nowMs) noexcept;
    void WriteSnapshotNow(
        const std::wstring& exeDir,
        const std::wstring& fileName,
        const std::string& stateJson) const noexcept;
    void TryWriteSnapshot(const std::wstring& exeDir, const std::string& stateJson, uint64_t nowMs) noexcept;

private:
    static constexpr uint64_t kDumpIntervalMs = 1500;
    std::atomic<uint64_t> lastDumpMs_{0};
};

} // namespace mousefx
