[CmdletBinding()]
param(
    [string]$BaseUrl = "",
    [string]$Token = "",
    [string]$RuntimeFile = "",
    [string]$WasmManifestPath = "",
    [string]$JsonOutput = "",
    [switch]$Help
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Show-Usage {
    @'
Usage:
  tools\platform\manual\run-windows-mouse-companion-renderer-lane-matrix.cmd [options]

Options:
  -BaseUrl <url>            Optional API base URL; falls back to runtime handoff when omitted
  -Token <token>            Optional x-mfcmouseeffect-token; falls back to runtime handoff when omitted
  -RuntimeFile <path>       Optional runtime handoff json path
  -WasmManifestPath <path>  Optional wasm manifest path; falls back to env when omitted
  -JsonOutput <path>        Optional output prefix; writes one json per lane plus summary files
  -Help                     Show this help
'@
}

function Fail([string]$Message) {
    Write-Error "[mfx:fail] $Message"
    exit 1
}

function Write-Ok([string]$Message) {
    Write-Host "[mfx:ok] $Message"
}

function Show-RendererLaneMatrixHint {
    @'
[mfx:info] renderer lane matrix
  - lane 1: builtin
    - reference baseline with no renderer sidecar contract attached
    - use this as the control group for motion / mood / silhouette comparison
  - lane 2: builtin_passthrough
    - same wasm attach path, but renderer semantics still come from builtin host
    - checked-in sample should feel a bit dreamier / lighter / more elastic than builtin
  - lane 3: wasm_v1
    - first bounded renderer-owned semantics patch lane
    - checked-in sample should read cooler / tighter / slightly more agile than builtin
  - recommended visual compare order:
    - follow: lift height, ear spread, tail swing, overall lightness
    - drag: lean direction, reach posture, eye focus
    - click / hold: squash, rebound, blush/highlight strength
    - scroll: tail lift and overall mood shift
  - suggested manual record:
    - builtin -> control
    - builtin_passthrough -> pass/fail + stronger or weaker than builtin
    - wasm_v1 -> pass/fail + stronger or weaker than builtin
'@ | Write-Host
}

function Resolve-DefaultOutputPrefix {
    $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
    return Join-Path ([System.IO.Path]::GetTempPath()) ("mfx-renderer-lane-matrix-{0}" -f $stamp)
}

function Resolve-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\..\.."))
}

function Resolve-WasmManifestPath([string]$ExplicitPath) {
    if (-not [string]::IsNullOrWhiteSpace($ExplicitPath)) {
        return [System.IO.Path]::GetFullPath($ExplicitPath)
    }
    if (-not [string]::IsNullOrWhiteSpace($env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST)) {
        return [System.IO.Path]::GetFullPath($env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST)
    }
    return ""
}

function Resolve-SidecarSamples {
    $repoRoot = Resolve-RepoRoot
    return [ordered]@{
        passthrough = (Join-Path $repoRoot "tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.sample.json")
        wasm_v1 = (Join-Path $repoRoot "tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.wasm-v1.sample.json")
    }
}

function Invoke-LaneProof(
    [string]$Label,
    [string]$Preset,
    [string]$BaseUrl,
    [string]$Token,
    [string]$RuntimeFile,
    [string]$JsonOutputPrefix) {
    $args = @(
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", (Join-Path $PSScriptRoot "run-windows-mouse-companion-render-proof.ps1"),
        "-Preset", $Preset
    )
    if (-not [string]::IsNullOrWhiteSpace($BaseUrl)) {
        $args += @("-BaseUrl", $BaseUrl)
    }
    if (-not [string]::IsNullOrWhiteSpace($Token)) {
        $args += @("-Token", $Token)
    }
    if (-not [string]::IsNullOrWhiteSpace($RuntimeFile)) {
        $args += @("-RuntimeFile", $RuntimeFile)
    }
    if (-not [string]::IsNullOrWhiteSpace($JsonOutputPrefix)) {
        $args += @("-JsonOutput", ("{0}.{1}.json" -f $JsonOutputPrefix, $Label))
    }
    & powershell @args
    return ($LASTEXITCODE -eq 0)
}

function Read-JsonFile([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path) -or -not (Test-Path -LiteralPath $Path)) {
        return $null
    }
    return Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
}

function New-LaneSummary(
    [string]$Label,
    [string]$JsonPath) {
    $json = Read-JsonFile $JsonPath
    if ($null -eq $json) {
        return [ordered]@{
            lane = $Label
            json_path = $JsonPath
            summary_status = "missing_json"
            lane_verdict = "missing_json"
            lane_brief = ("{0}: missing_json" -f $Label)
        }
    }

    $preview = $json.real_renderer_preview
    $results = @($json.results)
    $summaryNode = $json.summary
    $frameAdvanced = $null
    $expectationMet = $null
    if ($null -ne $summaryNode) {
        $frameAdvanced = $summaryNode.frame_advanced_count
        $expectationMet = $summaryNode.all_expectations_met
    } elseif ($null -ne $json.renderer_runtime_expectation_met) {
        $expectationMet = [bool]$json.renderer_runtime_expectation_met
        if ($null -ne $json.renderer_runtime_delta) {
            $frameAdvanced = $json.renderer_runtime_delta.frame_count_delta
        }
    }

    $pluginKind = if ($null -ne $preview) { [string]$preview.appearance_plugin_kind } else { "" }
    $semanticsMode = if ($null -ne $preview) { [string]$preview.appearance_plugin_appearance_semantics_mode } else { "" }
    $selectedBackend = [string]$json.selected_renderer_backend
    $expectationState = if ($expectationMet) { "pass" } else { "fail" }
    $laneVerdict = "{0}/{1}/{2}/{3}" -f $selectedBackend, $pluginKind, $semanticsMode, $expectationState

    return [ordered]@{
        lane = $Label
        json_path = $JsonPath
        summary_status = "ok"
        expectation_met = $expectationMet
        frame_signal = $frameAdvanced
        selected_backend = $selectedBackend
        preview_active = if ($null -ne $preview) { [bool]$preview.preview_active } else { $null }
        plugin_kind = $pluginKind
        semantics_mode = $semanticsMode
        combo_preset = if ($null -ne $preview) { [string]$preview.appearance_combo_preset } else { "" }
        selection_reason = if ($null -ne $preview) { [string]$preview.appearance_plugin_selection_reason } else { "" }
        failure_reason = if ($null -ne $preview) { [string]$preview.appearance_plugin_failure_reason } else { "" }
        metadata_path = if ($null -ne $preview) { [string]$preview.appearance_plugin_metadata_path } else { "" }
        lane_verdict = $laneVerdict
        lane_brief = ("{0}: {1}" -f $Label, $laneVerdict)
    }
}

function Compare-LaneAgainstBaseline(
    [hashtable]$Baseline,
    [hashtable]$Lane) {
    $diffs = New-Object System.Collections.Generic.List[string]
    if ($null -eq $Baseline -or $null -eq $Lane) {
        return [ordered]@{
            lane = if ($null -ne $Lane) { $Lane.lane } else { "" }
            baseline_lane = if ($null -ne $Baseline) { $Baseline.lane } else { "" }
            diff_count = 0
            diffs = @()
            compare_brief = "compare_unavailable"
        }
    }

    $comparisons = @(
        @{ name = "plugin_kind"; baseline = [string]$Baseline.plugin_kind; current = [string]$Lane.plugin_kind },
        @{ name = "semantics_mode"; baseline = [string]$Baseline.semantics_mode; current = [string]$Lane.semantics_mode },
        @{ name = "combo_preset"; baseline = [string]$Baseline.combo_preset; current = [string]$Lane.combo_preset },
        @{ name = "selection_reason"; baseline = [string]$Baseline.selection_reason; current = [string]$Lane.selection_reason },
        @{ name = "failure_reason"; baseline = [string]$Baseline.failure_reason; current = [string]$Lane.failure_reason },
        @{ name = "metadata_path_present"; baseline = (-not [string]::IsNullOrWhiteSpace([string]$Baseline.metadata_path)); current = (-not [string]::IsNullOrWhiteSpace([string]$Lane.metadata_path)) }
    )

    foreach ($item in $comparisons) {
        if ([string]$item.baseline -ne [string]$item.current) {
            $diffs.Add(("{0}: {1} -> {2}" -f $item.name, $item.baseline, $item.current))
        }
    }

    $brief = if ($diffs.Count -eq 0) {
        "same_as_builtin"
    } else {
        ($diffs | ForEach-Object { [string]$_ }) -join "; "
    }

    return [ordered]@{
        lane = $Lane.lane
        baseline_lane = $Baseline.lane
        diff_count = $diffs.Count
        diffs = @($diffs)
        compare_brief = $brief
    }
}

function New-LaneRecommendation(
    [object[]]$LaneSummaries,
    [object[]]$Comparisons) {
    $baseline = $LaneSummaries | Where-Object { $_.lane -eq "builtin" } | Select-Object -First 1
    $preferredOrder = @("wasm_v1", "builtin_passthrough")

    foreach ($laneName in $preferredOrder) {
        $lane = $LaneSummaries | Where-Object { $_.lane -eq $laneName } | Select-Object -First 1
        $comparison = $Comparisons | Where-Object { $_.lane -eq $laneName } | Select-Object -First 1
        if ($null -eq $lane) {
            continue
        }
        $laneOk = ($lane.summary_status -eq "ok") -and
            ($lane.expectation_met -eq $true) -and
            [string]::IsNullOrWhiteSpace([string]$lane.failure_reason)
        $hasMeaningfulDelta = ($null -ne $comparison) -and ($comparison.diff_count -gt 0)
        if ($laneOk -and $hasMeaningfulDelta) {
            return [ordered]@{
                recommended_default_lane = $laneName
                recommendation_reason = "machine_candidate:passed_and_differs_from_builtin"
                fallback_default_lane = if ($null -ne $baseline) { $baseline.lane } else { "builtin" }
                recommendation_confidence = "low"
                rollout_contract_status = "candidate_pending_manual_confirmation"
            }
        }
    }

    return [ordered]@{
        recommended_default_lane = if ($null -ne $baseline) { $baseline.lane } else { "builtin" }
        recommendation_reason = "machine_candidate:stay_on_builtin_until_manual_confirmation"
        fallback_default_lane = if ($null -ne $baseline) { $baseline.lane } else { "builtin" }
        recommendation_confidence = "low"
        rollout_contract_status = "stay_on_builtin"
    }
}

function Write-LaneMatrixSummary(
    [string]$OutputPrefix,
    [object[]]$LaneSummaries) {
    $summaryJsonPath = "{0}.summary.json" -f $OutputPrefix
    $summaryMdPath = "{0}.summary.md" -f $OutputPrefix
    $observationTemplatePath = "{0}.observation-template.md" -f $OutputPrefix

    $baselineLane = $LaneSummaries | Where-Object { $_.lane -eq "builtin" } | Select-Object -First 1
    $comparisons = @(
        $LaneSummaries |
            Where-Object { $_.lane -ne "builtin" } |
            ForEach-Object { Compare-LaneAgainstBaseline $baselineLane $_ }
    )
    $recommendation = New-LaneRecommendation $LaneSummaries $comparisons

    $payload = [ordered]@{
        generated_at = (Get-Date).ToString("s")
        lanes = $LaneSummaries
        comparisons_vs_builtin = $comparisons
        machine_recommendation = $recommendation
    }
    ($payload | ConvertTo-Json -Depth 8) | Set-Content -LiteralPath $summaryJsonPath -Encoding UTF8

    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add("# Renderer Lane Matrix Summary")
    $lines.Add("")
    $lines.Add(("- generated_at: `{0}`" -f $payload.generated_at))
    $lines.Add(("- summary_json: `{0}`" -f $summaryJsonPath))
    $lines.Add("")
    $lines.Add("## Lane Results")
    foreach ($lane in $LaneSummaries) {
        $lines.Add(("- `{0}` verdict: `{1}`" -f $lane.lane, $lane.lane_verdict))
        $lines.Add(("- `{0}`: expectation={1}, backend={2}, preview_active={3}, plugin_kind={4}, semantics_mode={5}, combo={6}" -f `
            $lane.lane,
            $lane.expectation_met,
            $lane.selected_backend,
            $lane.preview_active,
            $lane.plugin_kind,
            $lane.semantics_mode,
            $lane.combo_preset))
        $lines.Add(("  json: `{0}`" -f $lane.json_path))
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.selection_reason)) {
            $lines.Add(("  selection_reason: `{0}`" -f $lane.selection_reason))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.failure_reason)) {
            $lines.Add(("  failure_reason: `{0}`" -f $lane.failure_reason))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.metadata_path)) {
            $lines.Add(("  metadata_path: `{0}`" -f $lane.metadata_path))
        }
    }
    $lines.Add("")
    $lines.Add("## Compact Verdicts")
    foreach ($lane in $LaneSummaries) {
        $lines.Add(("- `{0}`" -f $lane.lane_brief))
    }
    $lines.Add("")
    $lines.Add("## Auto Compare vs builtin")
    foreach ($comparison in $comparisons) {
        $lines.Add(("- `{0}`: `{1}`" -f $comparison.lane, $comparison.compare_brief))
        foreach ($diff in @($comparison.diffs)) {
            $lines.Add(("  - `{0}`" -f $diff))
        }
    }
    $lines.Add("")
    $lines.Add("## Machine Recommendation")
    $lines.Add(("- recommended_default_lane: `{0}`" -f $recommendation.recommended_default_lane))
    $lines.Add(("- reason: `{0}`" -f $recommendation.recommendation_reason))
    $lines.Add(("- confidence: `{0}`" -f $recommendation.recommendation_confidence))
    $lines.Add(("- rollout_contract_status: `{0}`" -f $recommendation.rollout_contract_status))
    $lines.Add(("- note: machine recommendation is conservative and still needs manual observation confirmation"))
    $lines.Add("")
    $lines.Add("## Manual Compare")
    $lines.Add("- `builtin`: control baseline")
    $lines.Add("- `builtin_passthrough`: compare against builtin for dreamy/light/elastic deltas")
    $lines.Add("- `wasm_v1`: compare against builtin for cooler/tighter/agile deltas")
    $lines.Add("- focus on `follow / drag / click / hold / scroll`")
    $lines | Set-Content -LiteralPath $summaryMdPath -Encoding UTF8

    $observationLines = New-Object System.Collections.Generic.List[string]
    $observationLines.Add("# Renderer Lane Matrix Observation Template")
    $observationLines.Add("")
    $observationLines.Add(("- generated_at: `{0}`" -f $payload.generated_at))
    $observationLines.Add(("- summary_md: `{0}`" -f $summaryMdPath))
    $observationLines.Add("")
    $observationLines.Add("## Quick Verdict")
    $observationLines.Add("- `builtin_passthrough`: `pass|fail`, versus builtin = `stronger|weaker|same`")
    $observationLines.Add("- `wasm_v1`: `pass|fail`, versus builtin = `stronger|weaker|same`")
    $observationLines.Add("")
    $observationLines.Add("## Action Notes")
    $observationLines.Add("### follow")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    $observationLines.Add("- wasm_v1:")
    $observationLines.Add("- focus: lift height, ear spread, tail swing, overall lightness")
    $observationLines.Add("")
    $observationLines.Add("### drag")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    $observationLines.Add("- wasm_v1:")
    $observationLines.Add("- focus: lean direction, reach posture, eye/brow focus")
    $observationLines.Add("")
    $observationLines.Add("### click")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    $observationLines.Add("- wasm_v1:")
    $observationLines.Add("- focus: squash, rebound, blush/highlight strength")
    $observationLines.Add("")
    $observationLines.Add("### hold")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    $observationLines.Add("- wasm_v1:")
    $observationLines.Add("- focus: settle pulse, head nod, mood steadiness")
    $observationLines.Add("")
    $observationLines.Add("### scroll")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    $observationLines.Add("- wasm_v1:")
    $observationLines.Add("- focus: tail lift, glow/shadow/pedestal mood shift")
    $observationLines.Add("")
    $observationLines.Add("## Overall Call")
    $observationLines.Add("- lane readability: `pass|fail|pass (dynamic-biased lane delta)`")
    $observationLines.Add("- strongest lane:")
    $observationLines.Add("- weakest lane:")
    $observationLines.Add("- best lane for current Win pet:")
    $observationLines.Add(("- recommended default lane now: `{0}`" -f $recommendation.recommended_default_lane))
    $observationLines.Add(("- machine suggestion reason: `{0}`" -f $recommendation.recommendation_reason))
    $observationLines.Add(("- rollout contract status: `{0}`" -f $recommendation.rollout_contract_status))
    $observationLines.Add("- manual confirmation result: `approve_default_switch|reject_default_switch|needs_more_tuning`")
    $observationLines.Add("- recommended next tuning target:")
    $observationLines.Add("- notes:")
    $observationLines | Set-Content -LiteralPath $observationTemplatePath -Encoding UTF8

    Write-Host ("[mfx:info] renderer lane matrix summary json: {0}" -f $summaryJsonPath)
    Write-Host ("[mfx:info] renderer lane matrix summary md:   {0}" -f $summaryMdPath)
    Write-Host ("[mfx:info] renderer lane matrix notes md:    {0}" -f $observationTemplatePath)
}

if ($Help) {
    Show-Usage
    exit 0
}

$manifestPath = Resolve-WasmManifestPath $WasmManifestPath
$sidecarSamples = Resolve-SidecarSamples
if ([string]::IsNullOrWhiteSpace($JsonOutput)) {
    $JsonOutput = Resolve-DefaultOutputPrefix
}

if ([string]::IsNullOrWhiteSpace($manifestPath)) {
    Fail "missing wasm manifest path; pass -WasmManifestPath or set MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST"
}
if (-not (Test-Path -LiteralPath $manifestPath)) {
    Fail "wasm manifest not found: $manifestPath"
}
if (-not (Test-Path -LiteralPath $sidecarSamples.passthrough)) {
    Fail "passthrough sidecar sample not found: $($sidecarSamples.passthrough)"
}
if (-not (Test-Path -LiteralPath $sidecarSamples.wasm_v1)) {
    Fail "wasm_v1 sidecar sample not found: $($sidecarSamples.wasm_v1)"
}

$sidecarPath = [System.IO.Path]::ChangeExtension($manifestPath, ".mouse_companion_renderer.json")
$originalPluginMode = $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN
$originalManifestPath = $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST
$hadOriginalSidecar = Test-Path -LiteralPath $sidecarPath
$originalSidecarBytes = $null
if ($hadOriginalSidecar) {
    $originalSidecarBytes = [System.IO.File]::ReadAllBytes($sidecarPath)
}
$laneFailures = New-Object System.Collections.Generic.List[string]

try {
    Write-Host "[mfx:info] renderer lane matrix: builtin -> passthrough -> wasm_v1"
    Show-RendererLaneMatrixHint

    $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN = "builtin"
    $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST = $manifestPath
    if (Test-Path -LiteralPath $sidecarPath) {
        Remove-Item -LiteralPath $sidecarPath -Force
    }
    if (-not (Invoke-LaneProof "builtin" "real-preview-smoke" $BaseUrl $Token $RuntimeFile $JsonOutput)) {
        $laneFailures.Add("builtin")
    }

    $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN = "wasm"
    $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST = $manifestPath
    Copy-Item -LiteralPath $sidecarSamples.passthrough -Destination $sidecarPath -Force
    if (-not (Invoke-LaneProof "builtin_passthrough" "renderer-sidecar-smoke" $BaseUrl $Token $RuntimeFile $JsonOutput)) {
        $laneFailures.Add("builtin_passthrough")
    }

    Copy-Item -LiteralPath $sidecarSamples.wasm_v1 -Destination $sidecarPath -Force
    if (-not (Invoke-LaneProof "wasm_v1" "renderer-sidecar-wasm-v1-smoke" $BaseUrl $Token $RuntimeFile $JsonOutput)) {
        $laneFailures.Add("wasm_v1")
    }
} finally {
    if ($hadOriginalSidecar) {
        [System.IO.File]::WriteAllBytes($sidecarPath, $originalSidecarBytes)
    } elseif (Test-Path -LiteralPath $sidecarPath) {
        Remove-Item -LiteralPath $sidecarPath -Force
    }

    if ($null -eq $originalPluginMode) {
        Remove-Item Env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN -ErrorAction SilentlyContinue
    } else {
        $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN = $originalPluginMode
    }
    if ($null -eq $originalManifestPath) {
        Remove-Item Env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST -ErrorAction SilentlyContinue
    } else {
        $env:MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST = $originalManifestPath
    }
}

Write-LaneMatrixSummary $JsonOutput @(
    (New-LaneSummary "builtin" ("{0}.builtin.json" -f $JsonOutput)),
    (New-LaneSummary "builtin_passthrough" ("{0}.builtin_passthrough.json" -f $JsonOutput)),
    (New-LaneSummary "wasm_v1" ("{0}.wasm_v1.json" -f $JsonOutput))
)
if ($laneFailures.Count -gt 0) {
    Fail ("renderer lane matrix failed: {0}" -f (($laneFailures | ForEach-Object { [string]$_ }) -join ", "))
}
Write-Ok "renderer lane matrix"
