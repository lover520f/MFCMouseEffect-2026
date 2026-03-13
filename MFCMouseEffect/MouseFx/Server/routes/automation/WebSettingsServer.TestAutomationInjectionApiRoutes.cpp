#include "pch.h"
#include "WebSettingsServer.TestAutomationInjectionApiRoutes.h"

#include <string>
#include <vector>

#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Automation/GestureMatchSelection.h"
#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Input/GestureSimilarity.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/automation/WebSettingsServer.TestAutomationRouteUtils.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestRouteCommon.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
using automation_gesture_selection::Candidate;
using automation_gesture_selection::EffectiveRunnerUpScore;
using automation_gesture_selection::MeasureCandidate;
using automation_gesture_selection::PreferLeftOverRight;
using websettings_test_automation::BuildSelectedBindingJson;
using websettings_test_automation::IsAutomationInjectionTestApiEnabled;
using websettings_test_automation::IsAutomationScopeTestApiEnabled;
using websettings_test_automation::NormalizeMouseHistoryEntries;
using websettings_test_automation::ParseActionHistory;
using websettings_test_automation::ParseAutomationMappings;
using websettings_test_automation::ParseProcessBaseName;
using websettings_test_automation::ParseShortcutKeys;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

namespace {

std::vector<ScreenPoint> ParseScreenPointStroke(const json& value) {
    std::vector<ScreenPoint> stroke;
    if (!value.is_array()) {
        return stroke;
    }
    stroke.reserve(value.size());
    for (const auto& item : value) {
        if (!item.is_object()) {
            continue;
        }
        const int x = item.value("x", 0);
        const int y = item.value("y", 0);
        stroke.push_back(ScreenPoint{x, y});
    }
    return stroke;
}

std::vector<std::vector<ScreenPoint>> ParseScreenPointStrokes(const json& value) {
    std::vector<std::vector<ScreenPoint>> out;
    if (!value.is_array()) {
        return out;
    }
    out.reserve(value.size());
    for (const auto& strokeValue : value) {
        std::vector<ScreenPoint> stroke = ParseScreenPointStroke(strokeValue);
        if (!stroke.empty()) {
            out.push_back(std::move(stroke));
        }
    }
    return out;
}

std::vector<AutomationKeyBinding::GesturePoint> ParseGesturePointStroke(const json& value) {
    std::vector<AutomationKeyBinding::GesturePoint> stroke;
    if (!value.is_array()) {
        return stroke;
    }
    stroke.reserve(value.size());
    for (const auto& item : value) {
        if (!item.is_object()) {
            continue;
        }
        const int x = item.value("x", 0);
        const int y = item.value("y", 0);
        stroke.push_back(AutomationKeyBinding::GesturePoint{x, y});
    }
    return stroke;
}

std::vector<std::vector<AutomationKeyBinding::GesturePoint>> ParseTemplateStrokes(const json& value) {
    std::vector<std::vector<AutomationKeyBinding::GesturePoint>> out;
    if (!value.is_array()) {
        return out;
    }
    out.reserve(value.size());
    for (const auto& strokeValue : value) {
        std::vector<AutomationKeyBinding::GesturePoint> stroke = ParseGesturePointStroke(strokeValue);
        if (!stroke.empty()) {
            out.push_back(std::move(stroke));
        }
    }
    return out;
}

} // namespace

bool HandleWebSettingsTestAutomationInjectionApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/automation/test-match-and-inject") {
        if (!IsAutomationInjectionTestApiEnabled() || !IsAutomationScopeTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::vector<std::string> rawHistory = ParseActionHistory(payload);
        std::vector<std::string> normalizedHistory;
        const std::vector<automation_match::ActionHistoryEntry> history =
            NormalizeMouseHistoryEntries(rawHistory, &normalizedHistory);
        const std::vector<AutomationKeyBinding> mappings = ParseAutomationMappings(payload);

        std::string processBaseName = ParseProcessBaseName(payload);
        if (processBaseName.empty()) {
            processBaseName = controller->CurrentForegroundProcessBaseName();
        }
        const std::string normalizedProcess = automation_scope::NormalizeProcessName(processBaseName);

        const automation_match::BindingMatchResult match = automation_match::FindBestEnabledBinding(
            mappings,
            history,
            processBaseName,
            automation_match::ChainTimingLimit{},
            automation_ids::NormalizeMouseActionId);

        bool injected = false;
        if (match.binding != nullptr) {
            const std::string keys = TrimAscii(match.binding->keys);
            if (!keys.empty()) {
                injected = controller->InjectShortcutForTest(keys);
            }
        }

        const json selected = BuildSelectedBindingJson(match);
        SetJsonResponse(resp, json({
            {"ok", true},
            {"process", processBaseName},
            {"process_normalized", normalizedProcess},
            {"history", rawHistory},
            {"history_normalized", normalizedHistory},
            {"mapping_count", static_cast<uint64_t>(mappings.size())},
            {"matched", match.binding != nullptr},
            {"injected", injected},
            {"selected_binding_index", match.binding != nullptr ? static_cast<int64_t>(match.bindingIndex) : -1},
            {"selected_chain_length", static_cast<uint64_t>(match.chainLength)},
            {"selected_scope_specificity", match.scopeSpecificity},
            {"selected_keys", match.binding != nullptr ? match.binding->keys : std::string{}},
            {"selected", selected},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/test-inject-shortcut") {
        if (!IsAutomationInjectionTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::string keys = ParseShortcutKeys(payload);
        const bool accepted = !keys.empty() && controller->InjectShortcutForTest(keys);
        SetJsonResponse(resp, json({
            {"ok", true},
            {"keys", keys},
            {"accepted", accepted},
            {"process", controller->CurrentForegroundProcessBaseName()},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/test-gesture-similarity") {
        if (!IsAutomationScopeTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::vector<std::vector<ScreenPoint>> capturedStrokes =
            ParseScreenPointStrokes(payload.value("captured_strokes", json::array()));
        if (capturedStrokes.empty()) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "captured_strokes_empty"},
            }).dump());
            return true;
        }

        const json optionsValue = payload.value("options", json::object());
        GestureMatchOptions options{};
        options.enableWindowSearch = optionsValue.value("enable_window_search", true);
        options.strictStrokeCount = optionsValue.value("strict_stroke_count", true);
        options.strictStrokeOrder = optionsValue.value("strict_stroke_order", true);
        options.minEffectiveStrokeLengthPx = optionsValue.value("min_effective_stroke_length_px", 0.0);
        options.windowCoverageMinPercent = optionsValue.value("window_coverage_min_percent", 30);
        options.windowCoverageMaxPercent = optionsValue.value("window_coverage_max_percent", 100);
        options.windowCoverageStepPercent = optionsValue.value("window_coverage_step_percent", 12);
        options.windowSlideDivisor = optionsValue.value("window_slide_divisor", 4);
        options.enableTimeWindowSearch = optionsValue.value("enable_time_window_search", true);
        options.timeWindowMinMs = optionsValue.value("time_window_min_ms", 200);
        options.timeWindowMaxMs = optionsValue.value("time_window_max_ms", 1200);
        options.timeWindowStepMs = optionsValue.value("time_window_step_ms", 160);
        options.timeWindowAnchorStepMs = optionsValue.value("time_window_anchor_step_ms", 90);
        options.timeWindowMaxCandidates = optionsValue.value("time_window_max_candidates", 72);

        const double threshold = payload.value("threshold", 75.0);
        const double margin = payload.value("margin", 4.0);

        json evaluated = json::array();
        std::string bestId;
        double bestScore = -1.0;
        double runnerUpScore = -1.0;
        GestureMatchWindow bestWindow{};
        size_t bestCandidateCount = 0;
        GestureTemplateProfile bestProfile{};
        std::vector<std::pair<std::string, Candidate>> rankedCandidates;

        const json candidates = payload.value("candidates", json::array());
        for (const auto& candidate : candidates) {
            if (!candidate.is_object()) {
                continue;
            }
            const std::string id = TrimAscii(candidate.value("id", std::string{}));
            const std::string source = ToLowerAscii(TrimAscii(candidate.value("source", std::string{})));
            if (id.empty() || source.empty()) {
                continue;
            }

            GestureMatchResult result{};
            GestureTemplateProfile profile{};
            if (source == "preset") {
                const std::string actionId = automation_ids::NormalizeGestureId(
                    candidate.value("action_id", std::string{}));
                if (!actionId.empty() && capturedStrokes.size() == 1) {
                    result = MatchPresetGestureSimilarity(actionId, capturedStrokes.front(), options);
                    profile = MeasurePresetGestureProfile(actionId);
                }
            } else if (source == "custom") {
                const std::vector<std::vector<AutomationKeyBinding::GesturePoint>> templateStrokes =
                    ParseTemplateStrokes(candidate.value("template_strokes", json::array()));
                if (!templateStrokes.empty()) {
                    result = MatchGestureTemplateSimilarity(templateStrokes, capturedStrokes, options);
                    profile = MeasureGestureTemplateProfile(templateStrokes);
                }
            }

            size_t capturedPointCount = 0;
            for (const auto& stroke : capturedStrokes) {
                capturedPointCount += stroke.size();
            }
            const Candidate rankedCandidate{
                result.bestScore,
                result.runnerUpScore,
                result.bestWindow,
                result.candidateCount,
                profile,
                0,
                static_cast<int>(threshold),
                capturedPointCount,
            };
            const auto rankedMetrics = MeasureCandidate(rankedCandidate);

            evaluated.push_back({
                {"id", id},
                {"source", source},
                {"score", result.bestScore},
                {"selection_score", rankedMetrics.selectionScore},
                {"runner_up_score", result.runnerUpScore},
                {"candidate_count", result.candidateCount},
                {"best_window_start", static_cast<int64_t>(result.bestWindow.start)},
                {"best_window_end", static_cast<int64_t>(result.bestWindow.end)},
                {"profile_turns", static_cast<uint64_t>(profile.turnCount)},
                {"profile_segments", static_cast<uint64_t>(profile.segmentCount)},
                {"coverage_ratio", rankedMetrics.coverageRatio},
                {"valid", result.bestScore >= 0.0},
            });

            if (result.bestScore < 0.0) {
                continue;
            }
            rankedCandidates.push_back({id, rankedCandidate});
            if (bestId.empty() || PreferLeftOverRight(rankedCandidate, Candidate{
                bestScore,
                runnerUpScore,
                bestWindow,
                bestCandidateCount,
                bestProfile,
                0,
                static_cast<int>(threshold),
                capturedPointCount,
            })) {
                bestId = id;
                bestScore = result.bestScore;
                bestWindow = result.bestWindow;
                bestCandidateCount = result.candidateCount;
                bestProfile = profile;
            }
        }

        if (!bestId.empty()) {
            std::vector<Candidate> selectionPool;
            selectionPool.reserve(rankedCandidates.size());
            Candidate winner{};
            for (const auto& entry : rankedCandidates) {
                if (entry.first == bestId) {
                    winner = entry.second;
                    continue;
                }
                selectionPool.push_back(entry.second);
            }
            runnerUpScore = EffectiveRunnerUpScore(winner, selectionPool);
        }

        const bool hasWinner = !bestId.empty() && bestScore >= 0.0;
        const bool thresholdPassed = hasWinner && (bestScore + 1e-6 >= threshold);
        const bool marginPassed = hasWinner &&
            (runnerUpScore < 0.0 || bestScore - runnerUpScore + 1e-6 >= margin);
        const bool accepted = thresholdPassed && marginPassed;
        const std::string reason = !hasWinner
            ? "no_valid_candidate"
            : (!thresholdPassed ? "below_threshold" : (!marginPassed ? "ambiguous" : "accepted"));

        SetJsonResponse(resp, json({
            {"ok", true},
            {"accepted", accepted},
            {"reason", reason},
            {"threshold", threshold},
            {"margin", margin},
            {"best_id", bestId},
            {"best_score", bestScore},
            {"runner_up_score", runnerUpScore},
            {"best_window_start", static_cast<int64_t>(bestWindow.start)},
            {"best_window_end", static_cast<int64_t>(bestWindow.end)},
            {"best_candidate_count", static_cast<uint64_t>(bestCandidateCount)},
            {"candidates", evaluated},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
