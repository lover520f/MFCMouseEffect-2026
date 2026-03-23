[CmdletBinding()]
param(
    [string]$BaseUrl = "",
    [string]$Token = "",
    [string]$RuntimeFile = "",
    [string]$WasmManifestPath = "",
    [string]$JsonOutput = "",
    [ValidateSet("default", "agile", "dreamy", "charming")]
    [string]$WasmV1Style = "default",
    [switch]$AllWasmV1Styles,
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
  -WasmV1Style <style>      Optional `wasm_v1` sample style: default|agile|dreamy|charming (default: default)
  -AllWasmV1Styles          Run all checked-in `wasm_v1` styles: default, agile, dreamy, charming
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

function Show-RendererLaneMatrixHint(
    [string]$WasmV1Style,
    [bool]$AllWasmV1Styles) {
    $wasmHint = if ($AllWasmV1Styles) {
        "checked-in matrix will run default / agile / dreamy / charming variants in sequence"
    } else { switch ($WasmV1Style) {
        "default" { "checked-in sample should feel like the balanced default-candidate lane" }
        "dreamy" { "checked-in sample should feel brighter / softer / more floaty than builtin" }
        "charming" { "checked-in sample should feel warmer / rounder / more click-hold expressive than builtin" }
        default { "checked-in sample should read cooler / tighter / slightly more agile than builtin" }
    }}

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
    - WASM_V1_STYLE_HINT
  - recommended visual compare order:
    - follow: lift height, ear spread, tail swing, overall lightness
    - drag: lean direction, reach posture, eye focus
    - click / hold: squash, rebound, blush/highlight strength
    - scroll: tail lift and overall mood shift
  - suggested manual record:
    - builtin -> control
    - builtin_passthrough -> pass/fail + stronger or weaker than builtin
    - wasm_v1 -> pass/fail + stronger or weaker than builtin
'@.Replace("WASM_V1_STYLE_HINT", $wasmHint) | Write-Host
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
        wasm_v1_agile = (Join-Path $repoRoot "tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.wasm-v1.agile.sample.json")
        wasm_v1_default = (Join-Path $repoRoot "tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.wasm-v1.sample.json")
        wasm_v1_dreamy = (Join-Path $repoRoot "tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.wasm-v1.dreamy.sample.json")
        wasm_v1_charming = (Join-Path $repoRoot "tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.wasm-v1.charming.sample.json")
    }
}

function Resolve-WasmV1SamplePath(
    [hashtable]$SidecarSamples,
    [string]$WasmV1Style) {
    switch ($WasmV1Style) {
    "default" { return [string]$SidecarSamples.wasm_v1_default }
    "dreamy" { return [string]$SidecarSamples.wasm_v1_dreamy }
    "charming" { return [string]$SidecarSamples.wasm_v1_charming }
    default { return [string]$SidecarSamples.wasm_v1_agile }
    }
}

function Resolve-RecommendationSamplePath(
    [hashtable]$SidecarSamples,
    [string]$LaneName) {
    switch ($LaneName) {
    "wasm_v1_default" { return [string]$SidecarSamples.wasm_v1_default }
    "wasm_v1_agile" { return [string]$SidecarSamples.wasm_v1_agile }
    "wasm_v1_dreamy" { return [string]$SidecarSamples.wasm_v1_dreamy }
    "wasm_v1_charming" { return [string]$SidecarSamples.wasm_v1_charming }
    "wasm_v1" { return [string]$SidecarSamples.wasm_v1_default }
    "builtin_passthrough" { return [string]$SidecarSamples.passthrough }
    default { return "" }
    }
}

function New-WasmV1LaneSpecs(
    [hashtable]$SidecarSamples,
    [string]$WasmV1Style,
    [bool]$AllWasmV1Styles) {
    $styles = if ($AllWasmV1Styles) {
        @("default", "agile", "dreamy", "charming")
    } else {
        @($WasmV1Style)
    }

    $specs = New-Object System.Collections.Generic.List[object]
    foreach ($style in $styles) {
        $label = if ($AllWasmV1Styles) { "wasm_v1_{0}" -f $style } else { "wasm_v1" }
        $samplePath = Resolve-WasmV1SamplePath $SidecarSamples $style
        $sampleMetadata = Read-SidecarSampleMetadata $samplePath
        $specs.Add([ordered]@{
            label = $label
            style = $style
            sample_path = $samplePath
            sample_tier = [string]$sampleMetadata.sample_tier
        })
    }
    return @($specs)
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

function Read-SidecarSampleMetadata([string]$Path) {
    $fallback = [ordered]@{
        style_intent = ""
        sample_tier = ""
    }
    $json = Read-JsonFile $Path
    if ($null -eq $json) {
        return $fallback
    }
    return [ordered]@{
        style_intent = [string]$json.style_intent
        sample_tier = [string]$json.sample_tier
    }
}

function Format-DefaultLaneBrief(
    [string]$Candidate,
    [string]$Source,
    [string]$RolloutStatus,
    [string]$StyleIntent) {
    $candidateText = if ([string]::IsNullOrWhiteSpace($Candidate)) { "-" } else { $Candidate }
    $sourceText = if ([string]::IsNullOrWhiteSpace($Source)) { "-" } else { $Source }
    $rolloutText = if ([string]::IsNullOrWhiteSpace($RolloutStatus)) { "-" } else { $RolloutStatus }
    $styleText = if ([string]::IsNullOrWhiteSpace($StyleIntent)) { "-" } else { $StyleIntent }
    return "{0}/{1}/{2}/{3}" -f $candidateText, $sourceText, $rolloutText, $styleText
}

function Get-StyleIntentRecommendationRank([string]$StyleIntent) {
    switch ($StyleIntent) {
    "style_candidate:balanced_default_candidate" { return 600 }
    "style_candidate:agile_follow_drag" { return 500 }
    "style_candidate:dreamy_follow_scroll" { return 400 }
    "style_candidate:charming_click_hold" { return 300 }
    "style_candidate:single_selected_wasm_v1" { return 250 }
    "style_candidate:builtin_passthrough_baseline" { return 200 }
    default { return 0 }
    }
}

function Get-SampleTierRecommendationRank([string]$SampleTier) {
    switch ($SampleTier) {
    "ship_default_candidate" { return 1000 }
    "experimental_style_candidate" { return 500 }
    "baseline_reference" { return 200 }
    default { return 0 }
    }
}

function Get-DefaultLaneCandidateTierRecommendationRank([string]$CandidateTier) {
    switch ($CandidateTier) {
    "ship_default_candidate" { return 4000 }
    "experimental_style_candidate" { return 3000 }
    "baseline_reference_candidate" { return 2000 }
    "builtin_shipped_default" { return 1000 }
    default { return 0 }
    }
}

function Resolve-StyleFocusProfile(
    [string]$StyleIntent,
    [string]$LaneStyle) {
    switch ($StyleIntent) {
    "style_candidate:agile_follow_drag" { return "follow_drag_tension" }
    "style_candidate:dreamy_follow_scroll" { return "follow_scroll_float" }
    "style_candidate:charming_click_hold" { return "click_hold_warmth" }
    "style_candidate:balanced_default_candidate" { return "balanced_all_rounder" }
    "style_candidate:builtin_passthrough_baseline" { return "baseline_passthrough_reference" }
    }

    switch ($LaneStyle) {
    "builtin" { return "builtin_control" }
    "passthrough" { return "baseline_passthrough_reference" }
    "default" { return "balanced_all_rounder" }
    "agile" { return "follow_drag_tension" }
    "dreamy" { return "follow_scroll_float" }
    "charming" { return "click_hold_warmth" }
    default { return "unclassified_focus" }
    }
}

function New-LaneSummary(
    [string]$Label,
    [string]$JsonPath,
    [string]$ConfiguredStyle = "",
    [string]$ConfiguredSamplePath = "",
    [string]$ConfiguredSampleTier = "") {
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
    $defaultLaneCandidate = if ($null -ne $preview) { [string]$preview.default_lane_candidate } else { "" }
    $defaultLaneSource = if ($null -ne $preview) { [string]$preview.default_lane_source } else { "" }
    $defaultLaneRolloutStatus = if ($null -ne $preview) { [string]$preview.default_lane_rollout_status } else { "" }
    $defaultLaneStyleIntent = if ($null -ne $preview) { [string]$preview.default_lane_style_intent } else { "" }
    $defaultLaneCandidateTier = if ($null -ne $preview) { [string]$preview.default_lane_candidate_tier } else { "" }
    $runtimeSampleTier = if ($null -ne $preview) { [string]$preview.appearance_plugin_sample_tier } else { "" }
    $runtimeContractBrief = if ($null -ne $preview) {
        $existingContractBrief = [string]$preview.appearance_plugin_contract_brief
        if (-not [string]::IsNullOrWhiteSpace($existingContractBrief)) {
            $existingContractBrief
        } else {
            $mode = if ([string]::IsNullOrWhiteSpace($semanticsMode)) { "-" } else { $semanticsMode }
            $intent = if ([string]::IsNullOrWhiteSpace($defaultLaneStyleIntent)) { "-" } else { $defaultLaneStyleIntent }
            $tier = if ([string]::IsNullOrWhiteSpace($runtimeSampleTier)) { "-" } else { $runtimeSampleTier }
            "{0}/{1}/{2}" -f $mode, $intent, $tier
        }
    } else {
        ""
    }
    $runtimePoseAdapterBrief = if ($null -ne $preview) {
        $existingPoseBrief = [string]$preview.scene_runtime_pose_adapter_brief
        if (-not [string]::IsNullOrWhiteSpace($existingPoseBrief)) {
            $existingPoseBrief
        } else {
            $mode = [string]$preview.scene_runtime_adapter_mode
            if ([string]::IsNullOrWhiteSpace($mode)) { $mode = "runtime_only" }
            $influence = [double]([string]$preview.scene_runtime_pose_adapter_influence)
            $readability = [double]([string]$preview.scene_runtime_pose_readability_bias)
            "{0}/{1}/{2}" -f $mode, $influence.ToString("0.00"), $readability.ToString("0.00")
        }
    } else {
        ""
    }
    $runtimeModelSceneAdapterBrief = if ($null -ne $preview) {
        $existingModelSceneBrief = [string]$preview.scene_runtime_model_scene_adapter_brief
        if (-not [string]::IsNullOrWhiteSpace($existingModelSceneBrief)) {
            $existingModelSceneBrief
        } else {
            $state = [string]$preview.scene_runtime_model_scene_adapter_state
            if ([string]::IsNullOrWhiteSpace($state)) { $state = "preview_only" }
            $format = [string]$preview.model_source_format
            if ([string]::IsNullOrWhiteSpace($format)) { $format = "unknown" }
            $mode = [string]$preview.scene_runtime_adapter_mode
            if ([string]::IsNullOrWhiteSpace($mode)) { $mode = "runtime_only" }
            "{0}/{1}/{2}" -f $state, $format, $mode
        }
    } else {
        ""
    }
    $runtimeModelNodeAdapterBrief = if ($null -ne $preview) {
        $existingModelNodeBrief = [string]$preview.scene_runtime_model_node_adapter_brief
        if (-not [string]::IsNullOrWhiteSpace($existingModelNodeBrief)) {
            $existingModelNodeBrief
        } else {
            $influence = [double]([string]$preview.scene_runtime_model_node_adapter_influence)
            "preview_only/{0}" -f $influence.ToString("0.00")
        }
    } else {
        ""
    }
    $runtimeModelNodeChannelBrief = if ($null -ne $preview) {
        $existingModelNodeChannelBrief = [string]$preview.scene_runtime_model_node_channel_brief
        if (-not [string]::IsNullOrWhiteSpace($existingModelNodeChannelBrief)) {
            $existingModelNodeChannelBrief
        } else {
            "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
        }
    } else {
        ""
    }
    $runtimeModelNodeGraphBrief = if ($null -ne $preview) {
        $existingModelNodeGraphBrief = [string]$preview.scene_runtime_model_node_graph_brief
        if (-not [string]::IsNullOrWhiteSpace($existingModelNodeGraphBrief)) {
            $existingModelNodeGraphBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeModelNodeBindingBrief = if ($null -ne $preview) {
        $existingModelNodeBindingBrief = [string]$preview.scene_runtime_model_node_binding_brief
        if (-not [string]::IsNullOrWhiteSpace($existingModelNodeBindingBrief)) {
            $existingModelNodeBindingBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeModelNodeSlotBrief = if ($null -ne $preview) {
        $existingModelNodeSlotBrief = [string]$preview.scene_runtime_model_node_slot_brief
        if (-not [string]::IsNullOrWhiteSpace($existingModelNodeSlotBrief)) {
            $existingModelNodeSlotBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeModelNodeRegistryBrief = if ($null -ne $preview) {
        $existingModelNodeRegistryBrief = [string]$preview.scene_runtime_model_node_registry_brief
        if (-not [string]::IsNullOrWhiteSpace($existingModelNodeRegistryBrief)) {
            $existingModelNodeRegistryBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeBindingBrief = if ($null -ne $preview) {
        $existingAssetNodeBindingBrief = [string]$preview.scene_runtime_asset_node_binding_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeBindingBrief)) {
            $existingAssetNodeBindingBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeTransformBrief = if ($null -ne $preview) {
        $existingAssetNodeTransformBrief = [string]$preview.scene_runtime_asset_node_transform_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeTransformBrief)) {
            $existingAssetNodeTransformBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeAnchorBrief = if ($null -ne $preview) {
        $existingAssetNodeAnchorBrief = [string]$preview.scene_runtime_asset_node_anchor_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeAnchorBrief)) {
            $existingAssetNodeAnchorBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeResolverBrief = if ($null -ne $preview) {
        $existingAssetNodeResolverBrief = [string]$preview.scene_runtime_asset_node_resolver_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeResolverBrief)) {
            $existingAssetNodeResolverBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeParentSpaceBrief = if ($null -ne $preview) {
        $existingAssetNodeParentSpaceBrief = [string]$preview.scene_runtime_asset_node_parent_space_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeParentSpaceBrief)) {
            $existingAssetNodeParentSpaceBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeTargetBrief = if ($null -ne $preview) {
        $existingAssetNodeTargetBrief = [string]$preview.scene_runtime_asset_node_target_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeTargetBrief)) {
            $existingAssetNodeTargetBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeTargetResolverBrief = if ($null -ne $preview) {
        $existingAssetNodeTargetResolverBrief = [string]$preview.scene_runtime_asset_node_target_resolver_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeTargetResolverBrief)) {
            $existingAssetNodeTargetResolverBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodeWorldSpaceBrief = if ($null -ne $preview) {
        $existingAssetNodeWorldSpaceBrief = [string]$preview.scene_runtime_asset_node_world_space_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodeWorldSpaceBrief)) {
            $existingAssetNodeWorldSpaceBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $runtimeAssetNodePoseBrief = if ($null -ne $preview) {
        $existingAssetNodePoseBrief = [string]$preview.scene_runtime_asset_node_pose_brief
        if (-not [string]::IsNullOrWhiteSpace($existingAssetNodePoseBrief)) {
            $existingAssetNodePoseBrief
        } else {
            "preview_only/0/0"
        }
    } else {
        ""
    }
    $selectedBackend = [string]$json.selected_renderer_backend
    $expectationState = if ($expectationMet) { "pass" } else { "fail" }
    $laneVerdict = "{0}/{1}/{2}/{3}" -f $selectedBackend, $pluginKind, $semanticsMode, $expectationState
    $laneStyle = switch -Regex ($Label) {
        "^wasm_v1_default$" { "default" }
        "^wasm_v1_agile$" { "agile" }
        "^wasm_v1_dreamy$" { "dreamy" }
        "^wasm_v1_charming$" { "charming" }
        "^wasm_v1$" { "single" }
        "^builtin_passthrough$" { "passthrough" }
        "^builtin$" { "builtin" }
        default { "" }
    }

    return [ordered]@{
        lane = $Label
        style = $laneStyle
        style_focus_profile = (Resolve-StyleFocusProfile $defaultLaneStyleIntent $laneStyle)
        configured_style = $ConfiguredStyle
        configured_sample_path = $ConfiguredSamplePath
        configured_sample_tier = $ConfiguredSampleTier
        json_path = $JsonPath
        summary_status = "ok"
        expectation_met = $expectationMet
        frame_signal = $frameAdvanced
        selected_backend = $selectedBackend
        preview_active = if ($null -ne $preview) { [bool]$preview.preview_active } else { $null }
        plugin_kind = $pluginKind
        semantics_mode = $semanticsMode
        default_lane_candidate = $defaultLaneCandidate
        default_lane_source = $defaultLaneSource
        default_lane_rollout_status = $defaultLaneRolloutStatus
        default_lane_style_intent = $defaultLaneStyleIntent
        default_lane_candidate_tier = $defaultLaneCandidateTier
        runtime_sample_tier = $runtimeSampleTier
        runtime_contract_brief = $runtimeContractBrief
        runtime_model_scene_adapter_brief = $runtimeModelSceneAdapterBrief
        runtime_model_node_adapter_brief = $runtimeModelNodeAdapterBrief
        runtime_model_node_channel_brief = $runtimeModelNodeChannelBrief
        runtime_model_node_graph_brief = $runtimeModelNodeGraphBrief
        runtime_model_node_binding_brief = $runtimeModelNodeBindingBrief
        runtime_model_node_slot_brief = $runtimeModelNodeSlotBrief
        runtime_model_node_registry_brief = $runtimeModelNodeRegistryBrief
        runtime_asset_node_binding_brief = $runtimeAssetNodeBindingBrief
        runtime_asset_node_transform_brief = $runtimeAssetNodeTransformBrief
        runtime_asset_node_anchor_brief = $runtimeAssetNodeAnchorBrief
        runtime_asset_node_resolver_brief = $runtimeAssetNodeResolverBrief
        runtime_asset_node_parent_space_brief = $runtimeAssetNodeParentSpaceBrief
        runtime_asset_node_target_brief = $runtimeAssetNodeTargetBrief
        runtime_asset_node_target_resolver_brief = $runtimeAssetNodeTargetResolverBrief
        runtime_asset_node_world_space_brief = $runtimeAssetNodeWorldSpaceBrief
        runtime_asset_node_pose_brief = $runtimeAssetNodePoseBrief
        runtime_pose_adapter_brief = $runtimePoseAdapterBrief
        default_lane_brief = (Format-DefaultLaneBrief `
            $defaultLaneCandidate `
            $defaultLaneSource `
            $defaultLaneRolloutStatus `
            $defaultLaneStyleIntent)
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
        @{ name = "default_lane_candidate"; baseline = [string]$Baseline.default_lane_candidate; current = [string]$Lane.default_lane_candidate },
        @{ name = "default_lane_source"; baseline = [string]$Baseline.default_lane_source; current = [string]$Lane.default_lane_source },
        @{ name = "default_lane_rollout_status"; baseline = [string]$Baseline.default_lane_rollout_status; current = [string]$Lane.default_lane_rollout_status },
        @{ name = "default_lane_style_intent"; baseline = [string]$Baseline.default_lane_style_intent; current = [string]$Lane.default_lane_style_intent },
        @{ name = "default_lane_candidate_tier"; baseline = [string]$Baseline.default_lane_candidate_tier; current = [string]$Lane.default_lane_candidate_tier },
        @{ name = "runtime_sample_tier"; baseline = [string]$Baseline.runtime_sample_tier; current = [string]$Lane.runtime_sample_tier },
        @{ name = "runtime_contract_brief"; baseline = [string]$Baseline.runtime_contract_brief; current = [string]$Lane.runtime_contract_brief },
        @{ name = "runtime_model_scene_adapter_brief"; baseline = [string]$Baseline.runtime_model_scene_adapter_brief; current = [string]$Lane.runtime_model_scene_adapter_brief },
        @{ name = "runtime_model_node_adapter_brief"; baseline = [string]$Baseline.runtime_model_node_adapter_brief; current = [string]$Lane.runtime_model_node_adapter_brief },
        @{ name = "runtime_model_node_channel_brief"; baseline = [string]$Baseline.runtime_model_node_channel_brief; current = [string]$Lane.runtime_model_node_channel_brief },
        @{ name = "runtime_model_node_graph_brief"; baseline = [string]$Baseline.runtime_model_node_graph_brief; current = [string]$Lane.runtime_model_node_graph_brief },
        @{ name = "runtime_model_node_binding_brief"; baseline = [string]$Baseline.runtime_model_node_binding_brief; current = [string]$Lane.runtime_model_node_binding_brief },
        @{ name = "runtime_model_node_slot_brief"; baseline = [string]$Baseline.runtime_model_node_slot_brief; current = [string]$Lane.runtime_model_node_slot_brief },
        @{ name = "runtime_model_node_registry_brief"; baseline = [string]$Baseline.runtime_model_node_registry_brief; current = [string]$Lane.runtime_model_node_registry_brief },
        @{ name = "runtime_asset_node_binding_brief"; baseline = [string]$Baseline.runtime_asset_node_binding_brief; current = [string]$Lane.runtime_asset_node_binding_brief },
        @{ name = "runtime_asset_node_transform_brief"; baseline = [string]$Baseline.runtime_asset_node_transform_brief; current = [string]$Lane.runtime_asset_node_transform_brief },
        @{ name = "runtime_asset_node_anchor_brief"; baseline = [string]$Baseline.runtime_asset_node_anchor_brief; current = [string]$Lane.runtime_asset_node_anchor_brief },
        @{ name = "runtime_asset_node_resolver_brief"; baseline = [string]$Baseline.runtime_asset_node_resolver_brief; current = [string]$Lane.runtime_asset_node_resolver_brief },
        @{ name = "runtime_asset_node_parent_space_brief"; baseline = [string]$Baseline.runtime_asset_node_parent_space_brief; current = [string]$Lane.runtime_asset_node_parent_space_brief },
        @{ name = "runtime_asset_node_target_brief"; baseline = [string]$Baseline.runtime_asset_node_target_brief; current = [string]$Lane.runtime_asset_node_target_brief },
        @{ name = "runtime_asset_node_target_resolver_brief"; baseline = [string]$Baseline.runtime_asset_node_target_resolver_brief; current = [string]$Lane.runtime_asset_node_target_resolver_brief },
        @{ name = "runtime_asset_node_world_space_brief"; baseline = [string]$Baseline.runtime_asset_node_world_space_brief; current = [string]$Lane.runtime_asset_node_world_space_brief },
        @{ name = "runtime_asset_node_pose_brief"; baseline = [string]$Baseline.runtime_asset_node_pose_brief; current = [string]$Lane.runtime_asset_node_pose_brief },
        @{ name = "runtime_pose_adapter_brief"; baseline = [string]$Baseline.runtime_pose_adapter_brief; current = [string]$Lane.runtime_pose_adapter_brief },
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
    $candidates = New-Object System.Collections.Generic.List[object]
    foreach ($lane in $LaneSummaries) {
        if ($lane.lane -eq "builtin") {
            continue
        }
        $comparison = $Comparisons | Where-Object { $_.lane -eq $lane.lane } | Select-Object -First 1
        $laneOk = ($lane.summary_status -eq "ok") -and
            ($lane.expectation_met -eq $true) -and
            [string]::IsNullOrWhiteSpace([string]$lane.failure_reason)
        $hasMeaningfulDelta = ($null -ne $comparison) -and ($comparison.diff_count -gt 0)
        if (-not ($laneOk -and $hasMeaningfulDelta)) {
            continue
        }
        $styleIntent = [string]$lane.default_lane_style_intent
        if ([string]::IsNullOrWhiteSpace($styleIntent)) {
            $styleIntent = "style_candidate:none"
        }
        $runtimeSampleTier = [string]$lane.runtime_sample_tier
        $effectiveSampleTier = if ([string]::IsNullOrWhiteSpace($runtimeSampleTier)) {
            [string]$lane.configured_sample_tier
        } else {
            $runtimeSampleTier
        }
        $candidates.Add([ordered]@{
            lane = $lane
            comparison = $comparison
            style_intent = $styleIntent
            sample_tier = $effectiveSampleTier
            candidate_tier = [string]$lane.default_lane_candidate_tier
            candidate_tier_rank = (Get-DefaultLaneCandidateTierRecommendationRank ([string]$lane.default_lane_candidate_tier))
            tier_rank = (Get-SampleTierRecommendationRank $effectiveSampleTier)
            rank = (Get-StyleIntentRecommendationRank $styleIntent)
        })
    }

    $bestCandidate = $candidates |
        Sort-Object -Property @{ Expression = { $_.candidate_tier_rank }; Descending = $true }, @{ Expression = { $_.tier_rank }; Descending = $true }, @{ Expression = { $_.rank }; Descending = $true }, @{ Expression = { $_.comparison.diff_count }; Descending = $true } |
        Select-Object -First 1
    if ($null -ne $bestCandidate) {
        $lane = $bestCandidate.lane
        return [ordered]@{
            recommended_default_lane = $lane.lane
            recommendation_reason = "machine_candidate:passed_and_differs_from_builtin"
            recommendation_style_intent = $bestCandidate.style_intent
            recommendation_style_focus_profile = [string]$lane.style_focus_profile
            recommendation_candidate_tier = $bestCandidate.candidate_tier
            runtime_default_lane_brief = [string]$lane.default_lane_brief
            runtime_model_node_adapter_brief = [string]$lane.runtime_model_node_adapter_brief
            runtime_model_node_channel_brief = [string]$lane.runtime_model_node_channel_brief
            runtime_model_node_graph_brief = [string]$lane.runtime_model_node_graph_brief
            runtime_model_node_binding_brief = [string]$lane.runtime_model_node_binding_brief
            runtime_model_node_slot_brief = [string]$lane.runtime_model_node_slot_brief
            runtime_model_node_registry_brief = [string]$lane.runtime_model_node_registry_brief
            runtime_asset_node_binding_brief = [string]$lane.runtime_asset_node_binding_brief
            runtime_pose_adapter_brief = [string]$lane.runtime_pose_adapter_brief
            recommended_sample_path = [string]$lane.configured_sample_path
            recommended_sample_tier = [string]$bestCandidate.sample_tier
            fallback_default_lane = if ($null -ne $baseline) { $baseline.lane } else { "builtin" }
            recommendation_confidence = "low"
            rollout_contract_status = "candidate_pending_manual_confirmation"
        }
    }

    return [ordered]@{
        recommended_default_lane = if ($null -ne $baseline) { $baseline.lane } else { "builtin" }
        recommendation_reason = "machine_candidate:stay_on_builtin_until_manual_confirmation"
        recommendation_style_intent = "style_candidate:none"
        recommendation_style_focus_profile = if ($null -ne $baseline) { [string]$baseline.style_focus_profile } else { "builtin_control" }
        recommendation_candidate_tier = "builtin_shipped_default"
        runtime_default_lane_brief = if ($null -ne $baseline) { [string]$baseline.default_lane_brief } else { "builtin/runtime_builtin_default/stay_on_builtin/style_candidate:none" }
        runtime_model_node_adapter_brief = if ($null -ne $baseline) { [string]$baseline.runtime_model_node_adapter_brief } else { "preview_only/0.00" }
        runtime_model_node_channel_brief = if ($null -ne $baseline) { [string]$baseline.runtime_model_node_channel_brief } else { "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00" }
        runtime_model_node_graph_brief = if ($null -ne $baseline) { [string]$baseline.runtime_model_node_graph_brief } else { "preview_only/0/0" }
        runtime_model_node_binding_brief = if ($null -ne $baseline) { [string]$baseline.runtime_model_node_binding_brief } else { "preview_only/0/0" }
        runtime_model_node_slot_brief = if ($null -ne $baseline) { [string]$baseline.runtime_model_node_slot_brief } else { "preview_only/0/0" }
        runtime_model_node_registry_brief = if ($null -ne $baseline) { [string]$baseline.runtime_model_node_registry_brief } else { "preview_only/0/0" }
        runtime_asset_node_binding_brief = if ($null -ne $baseline) { [string]$baseline.runtime_asset_node_binding_brief } else { "preview_only/0/0" }
        runtime_pose_adapter_brief = if ($null -ne $baseline) { [string]$baseline.runtime_pose_adapter_brief } else { "runtime_only/0.00/0.00" }
        recommended_sample_path = ""
        recommended_sample_tier = ""
        fallback_default_lane = if ($null -ne $baseline) { $baseline.lane } else { "builtin" }
        recommendation_confidence = "low"
        rollout_contract_status = "stay_on_builtin"
    }
}

function Write-LaneMatrixSummary(
    [string]$OutputPrefix,
    [object[]]$LaneSummaries,
    [hashtable]$SidecarSamples,
    [string]$WasmV1Style,
    [bool]$AllWasmV1Styles) {
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
    $recommendationSamplePath = [string]$recommendation.recommended_sample_path
    if ([string]::IsNullOrWhiteSpace($recommendationSamplePath)) {
        $recommendationSamplePath =
            Resolve-RecommendationSamplePath $SidecarSamples $recommendation.recommended_default_lane
    }

    $payload = [ordered]@{
        generated_at = (Get-Date).ToString("s")
        lanes = $LaneSummaries
        comparisons_vs_builtin = $comparisons
        machine_recommendation = $recommendation
        recommended_sample_path = $recommendationSamplePath
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
        $lines.Add(("- `{0}`: style={1}, expectation={2}, backend={3}, preview_active={4}, plugin_kind={5}, semantics_mode={6}, default_lane_candidate={7}, combo={8}" -f `
            $lane.lane,
            $lane.style,
            $lane.expectation_met,
            $lane.selected_backend,
            $lane.preview_active,
            $lane.plugin_kind,
            $lane.semantics_mode,
            $lane.default_lane_candidate,
            $lane.combo_preset))
        $lines.Add(("  default_lane: `{0}`" -f $lane.default_lane_brief))
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.configured_style)) {
            $lines.Add(("  configured_style: `{0}`" -f $lane.configured_style))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.style_focus_profile)) {
            $lines.Add(("  style_focus_profile: `{0}`" -f $lane.style_focus_profile))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.configured_sample_path)) {
            $lines.Add(("  configured_sample_path: `{0}`" -f $lane.configured_sample_path))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.configured_sample_tier)) {
            $lines.Add(("  configured_sample_tier: `{0}`" -f $lane.configured_sample_tier))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_sample_tier)) {
            $lines.Add(("  runtime_sample_tier: `{0}`" -f $lane.runtime_sample_tier))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.default_lane_candidate_tier)) {
            $lines.Add(("  default_lane_candidate_tier: `{0}`" -f $lane.default_lane_candidate_tier))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_contract_brief)) {
            $lines.Add(("  runtime_contract_brief: `{0}`" -f $lane.runtime_contract_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_model_scene_adapter_brief)) {
            $lines.Add(("  runtime_model_scene_adapter_brief: `{0}`" -f $lane.runtime_model_scene_adapter_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_model_node_adapter_brief)) {
            $lines.Add(("  runtime_model_node_adapter_brief: `{0}`" -f $lane.runtime_model_node_adapter_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_model_node_channel_brief)) {
            $lines.Add(("  runtime_model_node_channel_brief: `{0}`" -f $lane.runtime_model_node_channel_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_model_node_graph_brief)) {
            $lines.Add(("  runtime_model_node_graph_brief: `{0}`" -f $lane.runtime_model_node_graph_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_model_node_binding_brief)) {
            $lines.Add(("  runtime_model_node_binding_brief: `{0}`" -f $lane.runtime_model_node_binding_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_model_node_slot_brief)) {
            $lines.Add(("  runtime_model_node_slot_brief: `{0}`" -f $lane.runtime_model_node_slot_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_model_node_registry_brief)) {
            $lines.Add(("  runtime_model_node_registry_brief: `{0}`" -f $lane.runtime_model_node_registry_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_binding_brief)) {
            $lines.Add(("  runtime_asset_node_binding_brief: `{0}`" -f $lane.runtime_asset_node_binding_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_transform_brief)) {
            $lines.Add(("  runtime_asset_node_transform_brief: `{0}`" -f $lane.runtime_asset_node_transform_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_anchor_brief)) {
            $lines.Add(("  runtime_asset_node_anchor_brief: `{0}`" -f $lane.runtime_asset_node_anchor_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_resolver_brief)) {
            $lines.Add(("  runtime_asset_node_resolver_brief: `{0}`" -f $lane.runtime_asset_node_resolver_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_parent_space_brief)) {
            $lines.Add(("  runtime_asset_node_parent_space_brief: `{0}`" -f $lane.runtime_asset_node_parent_space_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_target_brief)) {
            $lines.Add(("  runtime_asset_node_target_brief: `{0}`" -f $lane.runtime_asset_node_target_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_target_resolver_brief)) {
            $lines.Add(("  runtime_asset_node_target_resolver_brief: `{0}`" -f $lane.runtime_asset_node_target_resolver_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_world_space_brief)) {
            $lines.Add(("  runtime_asset_node_world_space_brief: `{0}`" -f $lane.runtime_asset_node_world_space_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_asset_node_pose_brief)) {
            $lines.Add(("  runtime_asset_node_pose_brief: `{0}`" -f $lane.runtime_asset_node_pose_brief))
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$lane.runtime_pose_adapter_brief)) {
            $lines.Add(("  runtime_pose_adapter_brief: `{0}`" -f $lane.runtime_pose_adapter_brief))
        }
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
    $lines.Add("## Default Lane Snapshots")
    foreach ($lane in $LaneSummaries) {
        $lines.Add(("- `{0}`: `{1}`" -f $lane.lane, $lane.default_lane_brief))
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
    $lines.Add(("- style_intent: `{0}`" -f $recommendation.recommendation_style_intent))
    $lines.Add(("- style_focus_profile: `{0}`" -f $recommendation.recommendation_style_focus_profile))
    $lines.Add(("- candidate_tier: `{0}`" -f $recommendation.recommendation_candidate_tier))
    $lines.Add(("- runtime_default_lane: `{0}`" -f $recommendation.runtime_default_lane_brief))
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_adapter_brief)) {
        $lines.Add(("- runtime_model_node_adapter_brief: `{0}`" -f $recommendation.runtime_model_node_adapter_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_channel_brief)) {
        $lines.Add(("- runtime_model_node_channel_brief: `{0}`" -f $recommendation.runtime_model_node_channel_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_graph_brief)) {
        $lines.Add(("- runtime_model_node_graph_brief: `{0}`" -f $recommendation.runtime_model_node_graph_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_binding_brief)) {
        $lines.Add(("- runtime_model_node_binding_brief: `{0}`" -f $recommendation.runtime_model_node_binding_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_slot_brief)) {
        $lines.Add(("- runtime_model_node_slot_brief: `{0}`" -f $recommendation.runtime_model_node_slot_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_registry_brief)) {
        $lines.Add(("- runtime_model_node_registry_brief: `{0}`" -f $recommendation.runtime_model_node_registry_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_asset_node_binding_brief)) {
        $lines.Add(("- runtime_asset_node_binding_brief: `{0}`" -f $recommendation.runtime_asset_node_binding_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_pose_adapter_brief)) {
        $lines.Add(("- runtime_pose_adapter_brief: `{0}`" -f $recommendation.runtime_pose_adapter_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace($recommendationSamplePath)) {
        $lines.Add(("- recommended_sample_path: `{0}`" -f $recommendationSamplePath))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.recommended_sample_tier)) {
        $lines.Add(("- recommended_sample_tier: `{0}`" -f $recommendation.recommended_sample_tier))
    }
    $lines.Add(("- confidence: `{0}`" -f $recommendation.recommendation_confidence))
    $lines.Add(("- rollout_contract_status: `{0}`" -f $recommendation.rollout_contract_status))
    $lines.Add(("- note: machine recommendation is conservative and still needs manual observation confirmation"))
    $lines.Add("")
    $lines.Add("## Manual Compare")
    $lines.Add("- `builtin`: control baseline")
    $lines.Add("- `builtin_passthrough`: compare against builtin for dreamy/light/elastic deltas")
    if ($AllWasmV1Styles) {
        $lines.Add("- `wasm_v1_*`: compare default / agile / dreamy / charming variants against builtin")
    } else {
        $lines.Add(("- `wasm_v1`: compare against builtin using `{0}` style" -f $WasmV1Style))
    }
    $lines.Add("- focus on `follow / drag / click / hold / scroll`")
    $lines | Set-Content -LiteralPath $summaryMdPath -Encoding UTF8

    $observationLines = New-Object System.Collections.Generic.List[string]
    $observationLines.Add("# Renderer Lane Matrix Observation Template")
    $observationLines.Add("")
    $observationLines.Add(("- generated_at: `{0}`" -f $payload.generated_at))
    $observationLines.Add(("- summary_md: `{0}`" -f $summaryMdPath))
    if ($AllWasmV1Styles) {
        $observationLines.Add("- wasm_v1_style: `all`")
    } else {
        $observationLines.Add(("- wasm_v1_style: `{0}`" -f $WasmV1Style))
    }
    $observationLines.Add("")
    $observationLines.Add("## Quick Verdict")
    $observationLines.Add("- `builtin_passthrough`: `pass|fail`, versus builtin = `stronger|weaker|same`")
    if ($AllWasmV1Styles) {
        $observationLines.Add("- `wasm_v1_default`: `pass|fail`, versus builtin = `stronger|weaker|same`")
        $observationLines.Add("- `wasm_v1_agile`: `pass|fail`, versus builtin = `stronger|weaker|same`")
        $observationLines.Add("- `wasm_v1_dreamy`: `pass|fail`, versus builtin = `stronger|weaker|same`")
        $observationLines.Add("- `wasm_v1_charming`: `pass|fail`, versus builtin = `stronger|weaker|same`")
    } else {
        $observationLines.Add(("- `wasm_v1 ({0})`: `pass|fail`, versus builtin = `stronger|weaker|same`" -f $WasmV1Style))
    }
    $observationLines.Add("")
    $observationLines.Add("## Action Notes")
    $observationLines.Add("### follow")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    if ($AllWasmV1Styles) {
        $observationLines.Add("- wasm_v1_default:")
        $observationLines.Add("- wasm_v1_agile:")
        $observationLines.Add("- wasm_v1_dreamy:")
        $observationLines.Add("- wasm_v1_charming:")
    } else {
        $observationLines.Add(("- wasm_v1 ({0}):" -f $WasmV1Style))
    }
    $observationLines.Add("- focus: lift height, ear spread, tail swing, overall lightness")
    $observationLines.Add("")
    $observationLines.Add("### drag")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    if ($AllWasmV1Styles) {
        $observationLines.Add("- wasm_v1_default:")
        $observationLines.Add("- wasm_v1_agile:")
        $observationLines.Add("- wasm_v1_dreamy:")
        $observationLines.Add("- wasm_v1_charming:")
    } else {
        $observationLines.Add(("- wasm_v1 ({0}):" -f $WasmV1Style))
    }
    $observationLines.Add("- focus: lean direction, reach posture, eye/brow focus")
    $observationLines.Add("")
    $observationLines.Add("### click")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    if ($AllWasmV1Styles) {
        $observationLines.Add("- wasm_v1_default:")
        $observationLines.Add("- wasm_v1_agile:")
        $observationLines.Add("- wasm_v1_dreamy:")
        $observationLines.Add("- wasm_v1_charming:")
    } else {
        $observationLines.Add(("- wasm_v1 ({0}):" -f $WasmV1Style))
    }
    $observationLines.Add("- focus: squash, rebound, blush/highlight strength")
    $observationLines.Add("")
    $observationLines.Add("### hold")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    if ($AllWasmV1Styles) {
        $observationLines.Add("- wasm_v1_default:")
        $observationLines.Add("- wasm_v1_agile:")
        $observationLines.Add("- wasm_v1_dreamy:")
        $observationLines.Add("- wasm_v1_charming:")
    } else {
        $observationLines.Add(("- wasm_v1 ({0}):" -f $WasmV1Style))
    }
    $observationLines.Add("- focus: settle pulse, head nod, mood steadiness")
    $observationLines.Add("")
    $observationLines.Add("### scroll")
    $observationLines.Add("- builtin:")
    $observationLines.Add("- builtin_passthrough:")
    if ($AllWasmV1Styles) {
        $observationLines.Add("- wasm_v1_default:")
        $observationLines.Add("- wasm_v1_agile:")
        $observationLines.Add("- wasm_v1_dreamy:")
        $observationLines.Add("- wasm_v1_charming:")
    } else {
        $observationLines.Add(("- wasm_v1 ({0}):" -f $WasmV1Style))
    }
    $observationLines.Add("- focus: tail lift, glow/shadow/pedestal mood shift")
    $observationLines.Add("")
    $observationLines.Add("## Overall Call")
    $observationLines.Add("- lane readability: `pass|fail|pass (dynamic-biased lane delta)`")
    $observationLines.Add("- strongest lane:")
    $observationLines.Add("- weakest lane:")
    $observationLines.Add("- best lane for current Win pet:")
    $observationLines.Add(("- recommended default lane now: `{0}`" -f $recommendation.recommended_default_lane))
    $observationLines.Add(("- machine suggestion reason: `{0}`" -f $recommendation.recommendation_reason))
    $observationLines.Add(("- machine style intent: `{0}`" -f $recommendation.recommendation_style_intent))
    $observationLines.Add(("- machine style focus: `{0}`" -f $recommendation.recommendation_style_focus_profile))
    $observationLines.Add(("- machine candidate tier: `{0}`" -f $recommendation.recommendation_candidate_tier))
    $observationLines.Add(("- machine runtime default lane: `{0}`" -f $recommendation.runtime_default_lane_brief))
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_adapter_brief)) {
        $observationLines.Add(("- machine model node adapter: `{0}`" -f $recommendation.runtime_model_node_adapter_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_channel_brief)) {
        $observationLines.Add(("- machine model node channels: `{0}`" -f $recommendation.runtime_model_node_channel_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_graph_brief)) {
        $observationLines.Add(("- machine model node graph: `{0}`" -f $recommendation.runtime_model_node_graph_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_binding_brief)) {
        $observationLines.Add(("- machine model node binding: `{0}`" -f $recommendation.runtime_model_node_binding_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_slot_brief)) {
        $observationLines.Add(("- machine model node slots: `{0}`" -f $recommendation.runtime_model_node_slot_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_model_node_registry_brief)) {
        $observationLines.Add(("- machine model node registry: `{0}`" -f $recommendation.runtime_model_node_registry_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_asset_node_binding_brief)) {
        $observationLines.Add(("- machine asset node binding: `{0}`" -f $recommendation.runtime_asset_node_binding_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.runtime_pose_adapter_brief)) {
        $observationLines.Add(("- machine pose adapter: `{0}`" -f $recommendation.runtime_pose_adapter_brief))
    }
    if (-not [string]::IsNullOrWhiteSpace($recommendationSamplePath)) {
        $observationLines.Add(("- machine recommended sample: `{0}`" -f $recommendationSamplePath))
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$recommendation.recommended_sample_tier)) {
        $observationLines.Add(("- machine recommended sample tier: `{0}`" -f $recommendation.recommended_sample_tier))
    }
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
$wasmV1LaneSpecs = New-WasmV1LaneSpecs $sidecarSamples $WasmV1Style $AllWasmV1Styles
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
foreach ($laneSpec in $wasmV1LaneSpecs) {
    if (-not (Test-Path -LiteralPath ([string]$laneSpec.sample_path))) {
        Fail "wasm_v1 sidecar sample not found: $([string]$laneSpec.sample_path)"
    }
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
    if ($AllWasmV1Styles) {
        Write-Host "[mfx:info] renderer lane matrix: builtin -> passthrough -> wasm_v1_default -> wasm_v1_agile -> wasm_v1_dreamy -> wasm_v1_charming"
    } else {
        Write-Host ("[mfx:info] renderer lane matrix: builtin -> passthrough -> wasm_v1 ({0})" -f $WasmV1Style)
    }
    Show-RendererLaneMatrixHint $WasmV1Style $AllWasmV1Styles

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

    foreach ($laneSpec in $wasmV1LaneSpecs) {
        Copy-Item -LiteralPath ([string]$laneSpec.sample_path) -Destination $sidecarPath -Force
        if (-not (Invoke-LaneProof ([string]$laneSpec.label) "renderer-sidecar-wasm-v1-smoke" $BaseUrl $Token $RuntimeFile $JsonOutput)) {
            $laneFailures.Add([string]$laneSpec.label)
        }
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

$laneSummaries = New-Object System.Collections.Generic.List[object]
$laneSummaries.Add((New-LaneSummary "builtin" ("{0}.builtin.json" -f $JsonOutput) "builtin" "" ""))
$laneSummaries.Add((New-LaneSummary "builtin_passthrough" ("{0}.builtin_passthrough.json" -f $JsonOutput) "passthrough" ([string]$sidecarSamples.passthrough) "baseline_reference"))
foreach ($laneSpec in $wasmV1LaneSpecs) {
    $laneSummaries.Add((New-LaneSummary ([string]$laneSpec.label) ("{0}.{1}.json" -f $JsonOutput, [string]$laneSpec.label) ([string]$laneSpec.style) ([string]$laneSpec.sample_path) ([string]$laneSpec.sample_tier)))
}

Write-LaneMatrixSummary $JsonOutput @($laneSummaries) $sidecarSamples $WasmV1Style $AllWasmV1Styles
if ($laneFailures.Count -gt 0) {
    Fail ("renderer lane matrix failed: {0}" -f (($laneFailures | ForEach-Object { [string]$_ }) -join ", "))
}
Write-Ok "renderer lane matrix"
