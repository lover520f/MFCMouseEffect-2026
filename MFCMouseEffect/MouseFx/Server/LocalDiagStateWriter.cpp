#include "pch.h"
#include "LocalDiagStateWriter.h"

#include <filesystem>
#include <fstream>

namespace mousefx {

bool LocalDiagStateWriter::ShouldWriteSnapshot(uint64_t nowMs) noexcept {
    const uint64_t prev = lastDumpMs_.load(std::memory_order_relaxed);
    if (nowMs >= prev && (nowMs - prev) < kDumpIntervalMs) {
        return false;
    }
    lastDumpMs_.store(nowMs, std::memory_order_relaxed);
    return true;
}

void LocalDiagStateWriter::WriteSnapshotNow(
    const std::wstring& exeDir,
    const std::wstring& fileName,
    const std::string& stateJson) const noexcept {
    if (exeDir.empty()) {
        return;
    }

    try {
        const std::filesystem::path dir = std::filesystem::path(exeDir) / L".local" / L"diag";
        const std::filesystem::path file = dir / (fileName.empty() ? L"web_state_auto.json" : fileName);
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            return;
        }

        std::ofstream out(file, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            return;
        }

        out.write(stateJson.data(), static_cast<std::streamsize>(stateJson.size()));
        out.flush();
    } catch (...) {
        return;
    }
}

void LocalDiagStateWriter::TryWriteSnapshot(const std::wstring& exeDir, const std::string& stateJson, uint64_t nowMs) noexcept {
    if (!ShouldWriteSnapshot(nowMs)) {
        return;
    }
    WriteSnapshotNow(exeDir, L"web_state_auto.json", stateJson);
}

} // namespace mousefx
