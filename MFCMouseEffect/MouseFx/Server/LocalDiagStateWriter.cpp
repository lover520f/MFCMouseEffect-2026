#include "pch.h"
#include "LocalDiagStateWriter.h"

#include <filesystem>
#include <fstream>

namespace mousefx {

void LocalDiagStateWriter::TryWriteSnapshot(const std::wstring& exeDir, const std::string& stateJson, uint64_t nowMs) noexcept {
    constexpr uint64_t kDumpIntervalMs = 1500;
    const uint64_t prev = lastDumpMs_.load(std::memory_order_relaxed);
    if (nowMs >= prev && (nowMs - prev) < kDumpIntervalMs) {
        return;
    }
    lastDumpMs_.store(nowMs, std::memory_order_relaxed);

    if (exeDir.empty()) {
        return;
    }

    try {
        const std::filesystem::path dir = std::filesystem::path(exeDir) / L".local" / L"diag";
        const std::filesystem::path file = dir / L"web_state_auto.json";
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

} // namespace mousefx

