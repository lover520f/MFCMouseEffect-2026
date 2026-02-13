#include "pch.h"

#include "GpuTakeoverControl.h"

#include <filesystem>
#include <fstream>

namespace mousefx::gpu {
namespace {

bool IsTruthyLeadingChar(wchar_t ch) {
    return ch == L'1' || ch == L't' || ch == L'T' || ch == L'y' || ch == L'Y';
}

bool IsVisibleTrialEnabledByFile(const std::filesystem::path& diagDir) {
    if (diagDir.empty()) return false;
    const std::filesystem::path onFile = diagDir / L"gpu_final_present_takeover.visible_trial.on";
    std::error_code ec;
    return std::filesystem::exists(onFile, ec) && !ec;
}

bool ConsumeRearmRequest(const std::filesystem::path& diagDir) {
    if (diagDir.empty()) return false;
    const std::filesystem::path rearmFile = diagDir / L"gpu_final_present_takeover.rearm";
    std::error_code ec;
    if (!std::filesystem::exists(rearmFile, ec) || ec) return false;

    const std::filesystem::path autoOffFile = diagDir / L"gpu_final_present_takeover.off.disabled_by_codex";
    ec.clear();
    std::filesystem::remove(autoOffFile, ec);
    ec.clear();
    std::filesystem::remove(rearmFile, ec);
    return true;
}

bool ConsumeOneShotEnableRequest(const std::filesystem::path& diagDir) {
    if (diagDir.empty()) return false;
    const std::filesystem::path onceFile = diagDir / L"gpu_final_present_takeover.once";
    std::error_code ec;
    if (!std::filesystem::exists(onceFile, ec) || ec) return false;
    ec.clear();
    std::filesystem::remove(onceFile, ec);
    return true;
}

bool ConsumeVisibleTrialOneShotEnableRequest(const std::filesystem::path& diagDir) {
    if (diagDir.empty()) return false;
    const std::filesystem::path onceFile = diagDir / L"gpu_final_present_takeover.visible_trial.once";
    std::error_code ec;
    if (!std::filesystem::exists(onceFile, ec) || ec) return false;
    ec.clear();
    std::filesystem::remove(onceFile, ec);
    return true;
}

void ArchiveStaleAutoOffMarker(const std::filesystem::path& autoOffFile) {
    if (autoOffFile.empty()) return;
    std::error_code ec;
    if (!std::filesystem::exists(autoOffFile, ec) || ec) return;
    const auto ts = GetTickCount64();
    std::filesystem::path archived = autoOffFile;
    archived += L".stale_ignored_";
    archived += std::to_wstring(ts);
    std::filesystem::rename(autoOffFile, archived, ec);
}

} // namespace

std::filesystem::path ResolveGpuDiagDirFromCurrentModule() {
    wchar_t modulePath[MAX_PATH]{};
    const DWORD n = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return {};
    return std::filesystem::path(modulePath).parent_path() / L".local" / L"diag";
}

void WriteGpuAutoDisableMarker(const char* reason) {
    const std::filesystem::path diagDir = ResolveGpuDiagDirFromCurrentModule();
    if (diagDir.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(diagDir, ec);
    if (ec) return;

    const std::filesystem::path marker = diagDir / L"gpu_final_present_takeover.off.disabled_by_codex";
    std::ofstream out(marker, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;
    out << (reason ? reason : "unknown");
}

TakeoverControlDecision ResolveTakeoverControlDecision() {
    TakeoverControlDecision result{};
    const std::filesystem::path diagDir = ResolveGpuDiagDirFromCurrentModule();
    const bool rearmed = ConsumeRearmRequest(diagDir);
    const bool visibleTrialOnceEnabled = ConsumeVisibleTrialOneShotEnableRequest(diagDir);
    const bool onceEnabled = ConsumeOneShotEnableRequest(diagDir);
    result.visibleTrialEnabled = IsVisibleTrialEnabledByFile(diagDir);
    result.visibleTrialFilePresent = result.visibleTrialEnabled;
    result.rearmProcessed = rearmed;
    result.onceFilePresent = onceEnabled;
    result.onceFileConsumed = onceEnabled;
    result.visibleTrialOnceFilePresent = visibleTrialOnceEnabled;
    result.visibleTrialOnceFileConsumed = visibleTrialOnceEnabled;
    if (visibleTrialOnceEnabled) {
        result.visibleTrialEnabled = true;
        result.visibleTrialFilePresent = true;
    }

    const std::filesystem::path exePath = []() -> std::filesystem::path {
        wchar_t modulePath[MAX_PATH]{};
        const DWORD n = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
        if (n == 0 || n >= MAX_PATH) return {};
        return std::filesystem::path(modulePath);
    }();

    if (!diagDir.empty()) {
        const std::filesystem::path offFile = diagDir / L"gpu_final_present_takeover.off";
        const std::filesystem::path onFile = diagDir / L"gpu_final_present_takeover.on";
        const std::filesystem::path autoOffFile = diagDir / L"gpu_final_present_takeover.off.disabled_by_codex";
        std::error_code ec;
        const bool offFilePresent = std::filesystem::exists(offFile, ec) && !ec;
        result.offFilePresent = offFilePresent;
        if (offFilePresent) {
            result.takeoverEnabled = false;
            result.source = "file_off";
            result.detail = "manual_off_file_present";
            return result;
        }
        ec.clear();
        const bool onFilePresent = std::filesystem::exists(onFile, ec) && !ec;
        result.onFilePresent = onFilePresent;
        if (onFilePresent) {
            result.takeoverEnabled = true;
            result.source = "file_on";
            result.detail = rearmed ? "rearmed_then_manual_on_file_present" : "manual_on_file_present";
            return result;
        }
        if (visibleTrialOnceEnabled) {
            result.takeoverEnabled = true;
            result.source = "file_visible_trial_once";
            result.detail = rearmed ? "rearmed_then_visible_trial_once_file_consumed" : "visible_trial_once_file_consumed";
            return result;
        }
        if (onceEnabled) {
            result.takeoverEnabled = true;
            result.source = "file_once";
            result.detail = rearmed ? "rearmed_then_once_file_consumed" : "once_file_consumed";
            return result;
        }
        ec.clear();
        const bool autoOffPresent = std::filesystem::exists(autoOffFile, ec) && !ec;
        result.autoOffFilePresent = autoOffPresent;
        if (autoOffPresent) {
            std::error_code fileEc;
            const auto autoOffTime = std::filesystem::last_write_time(autoOffFile, fileEc);
            std::error_code exeEc;
            const auto exeTime = exePath.empty() ? std::filesystem::file_time_type{} : std::filesystem::last_write_time(exePath, exeEc);
            if (!fileEc && !exeEc && !exePath.empty() && autoOffTime < exeTime) {
                ArchiveStaleAutoOffMarker(autoOffFile);
                result.autoOffFilePresent = false;
                result.takeoverEnabled = false;
                result.source = "auto_off_ignored_after_new_build";
                result.detail = "auto_off_marker_older_than_exe_archived";
            } else {
                result.takeoverEnabled = false;
                result.source = "file_off_auto";
                result.detail = rearmed ? "rearm_failed_auto_off_still_active" : "auto_off_marker_active";
                return result;
            }
        }
    }

    wchar_t value[8]{};
    const DWORD envN = GetEnvironmentVariableW(L"MOUSEFX_GPU_DCOMP_TAKEOVER", value, static_cast<DWORD>(std::size(value)));
    if (envN == 0 || envN >= std::size(value)) {
        if (rearmed) {
            result.detail = "rearmed_no_explicit_enable";
        }
        return result;
    }
    result.takeoverEnabled = IsTruthyLeadingChar(value[0]);
    result.source = "env";
    result.detail = result.takeoverEnabled
        ? (rearmed ? "rearmed_then_env_enabled" : "env_enabled")
        : (rearmed ? "rearmed_then_env_disabled" : "env_disabled");
    return result;
}

} // namespace mousefx::gpu
