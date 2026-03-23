[CmdletBinding()]
param(
    [string]$BaseUrl = "",
    [string]$Token = "",
    [string]$RuntimeFile = "",
    [string]$AppearanceProfilePath = "",
    [ValidateSet("proof", "sweep")]
    [string]$Route = "sweep",
    [string]$Preset = "",
    [string]$Event = "status",
    [int]$X = 640,
    [int]$Y = 360,
    [int]$Button = 1,
    [int]$Delta = 120,
    [int]$HoldMs = 420,
    [int]$WaitForFrameMs = 120,
    [bool]$ExpectFrameAdvance = $true,
    [string]$ExpectedBackend = "",
    [bool]$ExpectPreviewActive = $false,
    [bool]$ExpectAppearanceProfileMatch = $false,
    [string]$ExpectedAppearancePluginKind = "",
    [bool]$ExpectAppearancePluginMetadataPresent = $false,
    [string]$ExpectedAppearanceSemanticsMode = "",
    [string]$ExpectedDefaultLaneCandidate = "",
    [string]$ExpectedDefaultLaneSource = "",
    [string]$ExpectedDefaultLaneRolloutStatus = "",
    [string]$ExpectedDefaultLaneStyleIntent = "",
    [ValidateSet("default", "agile", "dreamy", "charming")]
    [string]$WasmV1Style = "default",
    [string]$JsonOutput = "",
    [switch]$Help
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Show-Usage {
    @'
Usage:
  tools\platform\manual\run-windows-mouse-companion-render-proof.cmd [options]

Options:
  -BaseUrl <url>               Required API base URL, e.g. http://127.0.0.1:8787
  -Token <token>               Required x-mfcmouseeffect-token value
  -RuntimeFile <path>          Optional runtime handoff json; auto-used when BaseUrl/Token are omitted
  -AppearanceProfilePath <p>   Optional pet-appearance.json path for appearance expectation checks
  -Route <proof|sweep>         Route kind (default: sweep)
  -Preset <name>               Named preset (currently: real-preview-smoke, combo-persona-acceptance, renderer-sidecar-smoke, renderer-sidecar-wasm-v1-smoke)
  -Event <name>                Single proof event when -Route proof (default: status)
  -X <int>                     Pointer x (default: 640)
  -Y <int>                     Pointer y (default: 360)
  -Button <int>                Button id (default: 1)
  -Delta <int>                 Scroll delta (default: 120)
  -HoldMs <int>                Hold duration (default: 420)
  -WaitForFrameMs <int>        Proof wait budget (default: 120)
  -ExpectFrameAdvance <bool>   Expect frame advance (default: true)
  -ExpectedBackend <name>      Require selected backend to match this name
  -ExpectPreviewActive <bool>  Require real preview active during proof (default: false)
  -ExpectAppearanceProfileMatch <bool>  Require renderer diagnostics to match pet-appearance.json
  -ExpectedAppearancePluginKind <name>  Require renderer plugin kind to match this name
  -ExpectAppearancePluginMetadataPresent <bool> Require non-empty sidecar metadata path
  -ExpectedAppearanceSemanticsMode <name> Require sidecar semantics mode to match this name
  -ExpectedDefaultLaneCandidate <name> Require runtime default lane candidate to match this name
  -ExpectedDefaultLaneSource <name> Require runtime default lane source to match this name
  -ExpectedDefaultLaneRolloutStatus <name> Require runtime default lane rollout status to match this name
  -ExpectedDefaultLaneStyleIntent <name> Require runtime default lane style intent to match this name
  -WasmV1Style <style>        Optional `wasm_v1` checked-in style for wasm-v1 smoke: default|agile|dreamy|charming
  -JsonOutput <path>           Save full JSON response to file
  -Help                        Show this help
'@
}

function Write-Ok([string]$Message) {
    Write-Host "[mfx:ok] $Message"
}

function Fail([string]$Message) {
    Write-Error "[mfx:fail] $Message"
    exit 1
}

function Resolve-WasmV1StyleIntent([string]$Style) {
    switch ($Style) {
    "agile" { return "style_candidate:agile_follow_drag" }
    "dreamy" { return "style_candidate:dreamy_follow_scroll" }
    "charming" { return "style_candidate:charming_click_hold" }
    default { return "style_candidate:balanced_default_candidate" }
    }
}

function Format-DefaultLaneSummary($Node) {
    if ($null -eq $Node) {
        return "-/-/-/-"
    }
    $candidate = [string]$Node.default_lane_candidate
    $source = [string]$Node.default_lane_source
    $rollout = [string]$Node.default_lane_rollout_status
    $styleIntent = [string]$Node.default_lane_style_intent
    if ([string]::IsNullOrWhiteSpace($candidate)) { $candidate = "-" }
    if ([string]::IsNullOrWhiteSpace($source)) { $source = "-" }
    if ([string]::IsNullOrWhiteSpace($rollout)) { $rollout = "-" }
    if ([string]::IsNullOrWhiteSpace($styleIntent)) { $styleIntent = "-" }
    return "{0}/{1}/{2}/{3}" -f $candidate, $source, $rollout, $styleIntent
}

function Add-DefaultLaneSummaryProperty($Node) {
    if ($null -eq $Node) {
        return
    }
    $summary = Format-DefaultLaneSummary $Node
    if ($Node.PSObject.Properties.Match("default_lane_summary").Count -gt 0) {
        $Node.default_lane_summary = $summary
        return
    }
    $Node | Add-Member -NotePropertyName "default_lane_summary" -NotePropertyValue $summary
}

function Show-RealPreviewSmokeHint {
    @'
[mfx:info] real-preview-smoke preset
  - expected env:
    - MFX_ENABLE_MOUSE_COMPANION_TEST_API=1
    - MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE=1
    - optional: MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND=real
  - expected gate:
    - selected backend should resolve to real
    - preview should be active
    - sweep should advance frames on action rows
'@ | Write-Host
}

function Show-ComboPersonaAcceptanceHint {
    @'
[mfx:info] combo-persona-acceptance preset
  - stage 1:
    - runs the same backend/preview/frame smoke gate as real-preview-smoke
  - stage 2:
    - manually compare these appearance combinations in the Windows app:
      - cream + moon
      - night + leaf
      - strawberry + ribbon-bow
  - expected reading:
    - cream + moon -> lighter / softer / dreamier
    - night + leaf -> tighter / more directional / more agile
    - strawberry + ribbon-bow -> sweeter / brighter / more charming
  - pass rule:
    - if static is weak but actions make the persona readable, record:
      - pass (dynamic-biased)
  - machine check:
    - also compares runtime appearance diagnostics against pet-appearance.json
'@ | Write-Host
}

function Show-RendererSidecarSmokeHint {
    @'
[mfx:info] renderer-sidecar-smoke preset
  - expected env:
    - MFX_ENABLE_MOUSE_COMPANION_TEST_API=1
    - MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE=1
    - MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN=wasm
    - MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST=<manifest path>
  - expected sidecar:
    - <manifest>.mouse_companion_renderer.json
    - appearance_semantics_mode=builtin_passthrough
  - expected gate:
    - selected backend should resolve to real
    - preview should be active
    - appearance_plugin_kind should resolve to wasm
    - appearance_plugin_metadata_path should be non-empty
    - appearance_plugin_appearance_semantics_mode should be builtin_passthrough
    - default_lane_candidate should be builtin_passthrough
    - default_lane_source should be env_wasm_candidate
    - default_lane_rollout_status should be candidate_pending_manual_confirmation
    - default_lane_style_intent should be style_candidate:builtin_passthrough_baseline
'@ | Write-Host
}

function Show-RendererSidecarWasmV1SmokeHint {
    @'
[mfx:info] renderer-sidecar-wasm-v1-smoke preset
  - expected env:
    - MFX_ENABLE_MOUSE_COMPANION_TEST_API=1
    - MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE=1
    - MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN=wasm
    - MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST=<manifest path>
  - expected sidecar:
    - <manifest>.mouse_companion_renderer.json
    - appearance_semantics_mode=wasm_v1
    - appearance_semantics object present
    - optional checked-in style: WASM_V1_STYLE
  - expected gate:
    - selected backend should resolve to real
    - preview should be active
    - appearance_plugin_kind should resolve to wasm
    - appearance_plugin_metadata_path should be non-empty
    - appearance_plugin_appearance_semantics_mode should be wasm_v1
    - default_lane_candidate should be wasm_v1
    - default_lane_source should be env_wasm_candidate
    - default_lane_rollout_status should be candidate_pending_manual_confirmation
    - default_lane_style_intent should match the selected wasm_v1 style intent
'@.Replace("WASM_V1_STYLE", $WasmV1Style) | Write-Host
}

function Resolve-DefaultRuntimeFile {
    $candidates = New-Object System.Collections.Generic.List[string]

    if (-not [string]::IsNullOrWhiteSpace($env:MFX_WEBSETTINGS_RUNTIME_FILE)) {
        $candidates.Add($env:MFX_WEBSETTINGS_RUNTIME_FILE)
    }
    if (-not [string]::IsNullOrWhiteSpace($env:APPDATA)) {
        $candidates.Add((Join-Path $env:APPDATA "MFCMouseEffect\websettings_runtime_auto.json"))
    }

    $repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\..\.."))
    $candidates.Add((Join-Path $repoRoot "x64\Release\websettings_runtime_auto.json"))
    $candidates.Add((Join-Path $repoRoot "x64\Debug\websettings_runtime_auto.json"))
    $candidates.Add((Join-Path $repoRoot "MFCMouseEffect\x64\Release\websettings_runtime_auto.json"))
    $candidates.Add((Join-Path $repoRoot "MFCMouseEffect\x64\Debug\websettings_runtime_auto.json"))

    foreach ($candidate in $candidates) {
        if ([string]::IsNullOrWhiteSpace($candidate)) {
            continue
        }
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    if ($candidates.Count -gt 0) {
        return $candidates[0]
    }
    return ""
}

function Resolve-DefaultAppearanceProfilePath {
    $repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\..\.."))
    $candidates = @(
        (Join-Path $repoRoot "MFCMouseEffect\Assets\Pet3D\source\pet-appearance.json"),
        (Join-Path $repoRoot "Assets\Pet3D\source\pet-appearance.json")
    )
    foreach ($candidate in $candidates) {
        if (-not [string]::IsNullOrWhiteSpace($candidate) -and (Test-Path -LiteralPath $candidate)) {
            return $candidate
        }
    }
    return $candidates[0]
}

function Resolve-ComboPersonaProfileCases {
    $repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\..\.."))
    return @(
        [ordered]@{
            label = "cream-moon"
            path = (Join-Path $repoRoot "MFCMouseEffect\Assets\Pet3D\source\pet-appearance.combo-cream-moon.json")
        },
        [ordered]@{
            label = "night-leaf"
            path = (Join-Path $repoRoot "MFCMouseEffect\Assets\Pet3D\source\pet-appearance.combo-night-leaf.json")
        },
        [ordered]@{
            label = "strawberry-ribbon-bow"
            path = (Join-Path $repoRoot "MFCMouseEffect\Assets\Pet3D\source\pet-appearance.combo-strawberry-ribbon-bow.json")
        }
    )
}

function Read-RuntimeHandoff([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $null
    }
    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }
    try {
        return Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
    } catch {
        Fail "failed to parse runtime handoff file: $Path"
    }
}

function Invoke-MfxApiJson(
    [string]$Method,
    [string]$Url,
    [hashtable]$Headers,
    [string]$Body) {
    if ([string]::IsNullOrWhiteSpace($Body)) {
        return Invoke-RestMethod -Method $Method -Uri $Url -Headers $Headers
    }
    return Invoke-RestMethod -Method $Method -Uri $Url -Headers $Headers -Body $Body
}

function Read-SettingsState([string]$BaseUrl, [string]$Token) {
    $headers = @{
        "x-mfcmouseeffect-token" = $Token
    }
    return Invoke-MfxApiJson "Get" ("{0}/api/state" -f $BaseUrl.TrimEnd("/")) $headers ""
}

function Apply-MouseCompanionAppearanceProfilePath(
    [string]$BaseUrl,
    [string]$Token,
    [string]$AppearancePath) {
    $headers = @{
        "x-mfcmouseeffect-token" = $Token
        "Content-Type" = "application/json"
    }
    $payload = @{
        mouse_companion = @{
            appearance_profile_path = $AppearancePath
        }
    } | ConvertTo-Json -Depth 8 -Compress
    return Invoke-MfxApiJson "Post" ("{0}/api/state" -f $BaseUrl.TrimEnd("/")) $headers $payload
}

function Resolve-AccessoryFamily([object[]]$AccessoryIds) {
    if ($null -eq $AccessoryIds) {
        return "none"
    }
    $ids = @($AccessoryIds | ForEach-Object { [string]$_ })
    foreach ($id in $ids) {
        if ($id -match "moon") { return "moon" }
    }
    foreach ($id in $ids) {
        if ($id -match "leaf") { return "leaf" }
    }
    foreach ($id in $ids) {
        if ($id -match "ribbon" -or $id -match "bow") { return "ribbon_bow" }
    }
    if ($ids.Count -gt 0) {
        return "star"
    }
    return "none"
}

function Resolve-ComboPreset([string]$SkinVariantId, [string]$AccessoryFamily) {
    if ($SkinVariantId -eq "cream" -and $AccessoryFamily -eq "moon") {
        return "dreamy"
    }
    if ($SkinVariantId -eq "night" -and $AccessoryFamily -eq "leaf") {
        return "agile"
    }
    if ($SkinVariantId -eq "strawberry" -and $AccessoryFamily -eq "ribbon_bow") {
        return "charming"
    }
    return "none"
}

function Resolve-ExpectedAppearance([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $null
    }
    if (-not (Test-Path -LiteralPath $Path)) {
        Fail "appearance profile not found: $Path"
    }

    try {
        $root = Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json -Depth 16
    } catch {
        Fail "failed to parse appearance profile: $Path"
    }

    $requestedPresetId = ""
    if ($null -ne $root.activePreset) {
        $requestedPresetId = [string]$root.activePreset
    }

    $resolvedNode = $null
    $resolvedPresetId = ""
    if (-not [string]::IsNullOrWhiteSpace($requestedPresetId) -and $null -ne $root.presets) {
        foreach ($preset in @($root.presets)) {
            if ($null -eq $preset) {
                continue
            }
            if ([string]$preset.id -eq $requestedPresetId) {
                $resolvedNode = $preset
                $resolvedPresetId = $requestedPresetId
                break
            }
        }
    }
    if ($null -eq $resolvedNode -and $null -ne $root.default) {
        $resolvedNode = $root.default
        $resolvedPresetId = "default"
    }
    if ($null -eq $resolvedNode) {
        Fail "appearance profile did not resolve to active preset or default: $Path"
    }

    $skinVariantId = [string]$resolvedNode.skinVariantId
    if ([string]::IsNullOrWhiteSpace($skinVariantId)) {
        $skinVariantId = "default"
    }
    $accessoryIds = @()
    if ($null -ne $resolvedNode.enabledAccessoryIds) {
        $accessoryIds = @($resolvedNode.enabledAccessoryIds | ForEach-Object { [string]$_ })
    }
    $accessoryFamily = Resolve-AccessoryFamily $accessoryIds
    $comboPreset = Resolve-ComboPreset $skinVariantId $accessoryFamily

    return [ordered]@{
        requested_preset_id = $requestedPresetId
        resolved_preset_id = $resolvedPresetId
        skin_variant_id = $skinVariantId
        accessory_ids = $accessoryIds
        accessory_family = $accessoryFamily
        combo_preset = $comboPreset
    }
}

function Compare-StringList([object]$Actual, [string[]]$Expected) {
    $actualList = @()
    if ($null -ne $Actual) {
        $actualList = @($Actual | ForEach-Object { [string]$_ })
    }
    if ($actualList.Count -ne $Expected.Count) {
        return $false
    }
    for ($i = 0; $i -lt $Expected.Count; $i++) {
        if ($actualList[$i] -ne $Expected[$i]) {
            return $false
        }
    }
    return $true
}

function Test-AppearanceExpectation([object]$Node, [hashtable]$ExpectedAppearance, [string]$Label) {
    if ($null -eq $ExpectedAppearance) {
        return @{ ok = $true; mismatches = @() }
    }
    if ($null -eq $Node) {
        return @{ ok = $false; mismatches = @("$Label:missing_node") }
    }

    $mismatches = New-Object System.Collections.Generic.List[string]
    if ([string]$Node.appearance_requested_preset_id -ne [string]$ExpectedAppearance.requested_preset_id) {
        $mismatches.Add(("{0}:requested_preset expected={1} actual={2}" -f $Label, $ExpectedAppearance.requested_preset_id, [string]$Node.appearance_requested_preset_id))
    }
    if ([string]$Node.appearance_resolved_preset_id -ne [string]$ExpectedAppearance.resolved_preset_id) {
        $mismatches.Add(("{0}:resolved_preset expected={1} actual={2}" -f $Label, $ExpectedAppearance.resolved_preset_id, [string]$Node.appearance_resolved_preset_id))
    }
    if ([string]$Node.appearance_skin_variant_id -ne [string]$ExpectedAppearance.skin_variant_id) {
        $mismatches.Add(("{0}:skin_variant expected={1} actual={2}" -f $Label, $ExpectedAppearance.skin_variant_id, [string]$Node.appearance_skin_variant_id))
    }
    if ([string]$Node.appearance_accessory_family -ne [string]$ExpectedAppearance.accessory_family) {
        $mismatches.Add(("{0}:accessory_family expected={1} actual={2}" -f $Label, $ExpectedAppearance.accessory_family, [string]$Node.appearance_accessory_family))
    }
    if ([string]$Node.appearance_combo_preset -ne [string]$ExpectedAppearance.combo_preset) {
        $mismatches.Add(("{0}:combo_preset expected={1} actual={2}" -f $Label, $ExpectedAppearance.combo_preset, [string]$Node.appearance_combo_preset))
    }
    if (-not (Compare-StringList $Node.appearance_accessory_ids $ExpectedAppearance.accessory_ids)) {
        $mismatches.Add(("{0}:accessory_ids expected=[{1}] actual=[{2}]" -f `
            $Label, `
            (($ExpectedAppearance.accessory_ids) -join ","), `
            ((@($Node.appearance_accessory_ids | ForEach-Object { [string]$_ })) -join ",")))
    }
    return @{
        ok = ($mismatches.Count -eq 0)
        mismatches = @($mismatches)
    }
}

function Test-RendererPluginExpectation(
    [object]$Node,
    [string]$ExpectedPluginKind,
    [bool]$ExpectMetadataPresent,
    [string]$ExpectedSemanticsMode,
    [string]$ExpectedDefaultLaneCandidate,
    [string]$ExpectedDefaultLaneSource,
    [string]$ExpectedDefaultLaneRolloutStatus,
    [string]$ExpectedDefaultLaneStyleIntent,
    [string]$Label) {
    $mismatches = New-Object System.Collections.Generic.List[string]
    if ($null -eq $Node) {
        $mismatches.Add("$Label:missing_node")
        return @{ ok = $false; mismatches = @($mismatches) }
    }
    if (-not [string]::IsNullOrWhiteSpace($ExpectedPluginKind) -and
        ([string]$Node.appearance_plugin_kind -ne $ExpectedPluginKind)) {
        $mismatches.Add(("{0}:plugin_kind expected={1} actual={2}" -f
            $Label, $ExpectedPluginKind, [string]$Node.appearance_plugin_kind))
    }
    if ($ExpectMetadataPresent -and
        [string]::IsNullOrWhiteSpace([string]$Node.appearance_plugin_metadata_path)) {
        $mismatches.Add(("{0}:metadata_path expected=non_empty actual=empty" -f $Label))
    }
    if (-not [string]::IsNullOrWhiteSpace($ExpectedSemanticsMode) -and
        ([string]$Node.appearance_plugin_appearance_semantics_mode -ne $ExpectedSemanticsMode)) {
        $mismatches.Add(("{0}:appearance_semantics_mode expected={1} actual={2}" -f
            $Label, $ExpectedSemanticsMode, [string]$Node.appearance_plugin_appearance_semantics_mode))
    }
    if (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneCandidate) -and
        ([string]$Node.default_lane_candidate -ne $ExpectedDefaultLaneCandidate)) {
        $mismatches.Add(("{0}:default_lane_candidate expected={1} actual={2}" -f
            $Label, $ExpectedDefaultLaneCandidate, [string]$Node.default_lane_candidate))
    }
    if (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneSource) -and
        ([string]$Node.default_lane_source -ne $ExpectedDefaultLaneSource)) {
        $mismatches.Add(("{0}:default_lane_source expected={1} actual={2}" -f
            $Label, $ExpectedDefaultLaneSource, [string]$Node.default_lane_source))
    }
    if (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneRolloutStatus) -and
        ([string]$Node.default_lane_rollout_status -ne $ExpectedDefaultLaneRolloutStatus)) {
        $mismatches.Add(("{0}:default_lane_rollout_status expected={1} actual={2}" -f
            $Label, $ExpectedDefaultLaneRolloutStatus, [string]$Node.default_lane_rollout_status))
    }
    if (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneStyleIntent) -and
        ([string]$Node.default_lane_style_intent -ne $ExpectedDefaultLaneStyleIntent)) {
        $mismatches.Add(("{0}:default_lane_style_intent expected={1} actual={2}" -f
            $Label, $ExpectedDefaultLaneStyleIntent, [string]$Node.default_lane_style_intent))
    }
    return @{
        ok = ($mismatches.Count -eq 0)
        mismatches = @($mismatches)
    }
}

if ($Help) {
    Show-Usage
    exit 0
}

$runtimeInfo = $null
$expectedAppearance = $null
$runComboPersonaMatrix = $false

switch ($Preset) {
    "" { }
    "real-preview-smoke" {
        $Route = "sweep"
        $WaitForFrameMs = 120
        $ExpectFrameAdvance = $true
        $ExpectedBackend = "real"
        $ExpectPreviewActive = $true
        Show-RealPreviewSmokeHint
    }
    "combo-persona-acceptance" {
        $Route = "sweep"
        $WaitForFrameMs = 120
        $ExpectFrameAdvance = $true
        $ExpectedBackend = "real"
        $ExpectPreviewActive = $true
        $ExpectAppearanceProfileMatch = $true
        $runComboPersonaMatrix = $true
        Show-RealPreviewSmokeHint
        Show-ComboPersonaAcceptanceHint
    }
    "renderer-sidecar-smoke" {
        $Route = "sweep"
        $WaitForFrameMs = 120
        $ExpectFrameAdvance = $true
        $ExpectedBackend = "real"
        $ExpectPreviewActive = $true
        $ExpectedAppearancePluginKind = "wasm"
        $ExpectAppearancePluginMetadataPresent = $true
        $ExpectedAppearanceSemanticsMode = "builtin_passthrough"
        $ExpectedDefaultLaneCandidate = "builtin_passthrough"
        $ExpectedDefaultLaneSource = "env_wasm_candidate"
        $ExpectedDefaultLaneRolloutStatus = "candidate_pending_manual_confirmation"
        $ExpectedDefaultLaneStyleIntent = "style_candidate:builtin_passthrough_baseline"
        Show-RealPreviewSmokeHint
        Show-RendererSidecarSmokeHint
    }
    "renderer-sidecar-wasm-v1-smoke" {
        $Route = "sweep"
        $WaitForFrameMs = 120
        $ExpectFrameAdvance = $true
        $ExpectedBackend = "real"
        $ExpectPreviewActive = $true
        $ExpectedAppearancePluginKind = "wasm"
        $ExpectAppearancePluginMetadataPresent = $true
        $ExpectedAppearanceSemanticsMode = "wasm_v1"
        $ExpectedDefaultLaneCandidate = "wasm_v1"
        $ExpectedDefaultLaneSource = "env_wasm_candidate"
        $ExpectedDefaultLaneRolloutStatus = "candidate_pending_manual_confirmation"
        $ExpectedDefaultLaneStyleIntent = Resolve-WasmV1StyleIntent $WasmV1Style
        Show-RealPreviewSmokeHint
        Show-RendererSidecarWasmV1SmokeHint
    }
    default {
        Fail "invalid -Preset value: $Preset (expected: real-preview-smoke, combo-persona-acceptance, renderer-sidecar-smoke, or renderer-sidecar-wasm-v1-smoke)"
    }
}

if (-not $runComboPersonaMatrix -and $ExpectAppearanceProfileMatch -and [string]::IsNullOrWhiteSpace($AppearanceProfilePath)) {
    $AppearanceProfilePath = Resolve-DefaultAppearanceProfilePath
}
if (-not $runComboPersonaMatrix -and $ExpectAppearanceProfileMatch) {
    $expectedAppearance = Resolve-ExpectedAppearance $AppearanceProfilePath
    Write-Host ("[mfx:info] appearance expectation requested={0} resolved={1} skin={2} accessory_family={3} combo={4}" -f `
        $expectedAppearance.requested_preset_id, `
        $expectedAppearance.resolved_preset_id, `
        $expectedAppearance.skin_variant_id, `
        $expectedAppearance.accessory_family, `
        $expectedAppearance.combo_preset)
}

if ([string]::IsNullOrWhiteSpace($BaseUrl)) {
    if ([string]::IsNullOrWhiteSpace($RuntimeFile)) {
        $RuntimeFile = Resolve-DefaultRuntimeFile
    }
    $runtimeInfo = Read-RuntimeHandoff $RuntimeFile
    if ($null -ne $runtimeInfo -and -not [string]::IsNullOrWhiteSpace($runtimeInfo.base_url)) {
        $BaseUrl = [string]$runtimeInfo.base_url
    }
}
if ([string]::IsNullOrWhiteSpace($Token)) {
    if ([string]::IsNullOrWhiteSpace($RuntimeFile)) {
        $RuntimeFile = Resolve-DefaultRuntimeFile
    }
    if ($null -eq $runtimeInfo) {
        $runtimeInfo = Read-RuntimeHandoff $RuntimeFile
    }
    if ($null -ne $runtimeInfo -and -not [string]::IsNullOrWhiteSpace($runtimeInfo.token)) {
        $Token = [string]$runtimeInfo.token
    }
}

if ([string]::IsNullOrWhiteSpace($BaseUrl)) {
    Fail "missing required -BaseUrl and no runtime handoff file could provide it"
}
if ([string]::IsNullOrWhiteSpace($Token)) {
    Fail "missing required -Token and no runtime handoff file could provide it"
}

if ($runComboPersonaMatrix) {
    $originalState = Read-SettingsState $BaseUrl $Token
    $originalAppearanceProfilePath = ""
    if ($null -ne $originalState -and $null -ne $originalState.mouse_companion) {
        $originalAppearanceProfilePath = [string]$originalState.mouse_companion.appearance_profile_path
    }
    $cases = Resolve-ComboPersonaProfileCases
    $failedCases = New-Object System.Collections.Generic.List[string]
    try {
        foreach ($case in $cases) {
            if (-not (Test-Path -LiteralPath $case.path)) {
                $failedCases.Add(("{0}:profile_missing" -f $case.label))
                Write-Host ("[mfx:fail] combo persona case {0}: missing profile {1}" -f $case.label, $case.path)
                continue
            }
            Write-Host ("[mfx:info] combo persona case {0}: applying {1}" -f $case.label, $case.path)
            Apply-MouseCompanionAppearanceProfilePath $BaseUrl $Token $case.path | Out-Null
            $childArgs = @(
                "-NoProfile",
                "-ExecutionPolicy", "Bypass",
                "-File", $PSCommandPath,
                "-BaseUrl", $BaseUrl,
                "-Token", $Token,
                "-AppearanceProfilePath", $case.path,
                "-Route", $Route,
                "-Event", $Event,
                "-X", $X,
                "-Y", $Y,
                "-Button", $Button,
                "-Delta", $Delta,
                "-HoldMs", $HoldMs,
                "-WaitForFrameMs", $WaitForFrameMs,
                "-ExpectFrameAdvance", [string]$ExpectFrameAdvance,
                "-ExpectedBackend", $ExpectedBackend,
                "-ExpectPreviewActive", [string]$ExpectPreviewActive,
                "-ExpectAppearanceProfileMatch", "true",
                "-ExpectedAppearancePluginKind", $ExpectedAppearancePluginKind,
                "-ExpectAppearancePluginMetadataPresent", [string]$ExpectAppearancePluginMetadataPresent,
                "-ExpectedAppearanceSemanticsMode", $ExpectedAppearanceSemanticsMode
            )
            if (-not [string]::IsNullOrWhiteSpace($RuntimeFile)) {
                $childArgs += @("-RuntimeFile", $RuntimeFile)
            }
            if (-not [string]::IsNullOrWhiteSpace($JsonOutput)) {
                $caseSafeLabel = ([string]$case.label) -replace '[^A-Za-z0-9._-]', '_'
                $childArgs += @("-JsonOutput", ("{0}.{1}.json" -f $JsonOutput, $caseSafeLabel))
            }
            & powershell @childArgs
            if ($LASTEXITCODE -ne 0) {
                $failedCases.Add([string]$case.label)
            }
        }
    } finally {
        if (-not [string]::IsNullOrWhiteSpace($originalAppearanceProfilePath)) {
            Write-Host ("[mfx:info] restoring appearance profile path: {0}" -f $originalAppearanceProfilePath)
            Apply-MouseCompanionAppearanceProfilePath $BaseUrl $Token $originalAppearanceProfilePath | Out-Null
        }
    }
    if ($failedCases.Count -gt 0) {
        Fail ("combo persona matrix failed: {0}" -f (($failedCases | ForEach-Object { [string]$_ }) -join ", "))
    }
    Write-Ok "combo persona matrix"
    exit 0
}

$endpoint = $BaseUrl.TrimEnd("/")
if ($Route -eq "sweep") {
    $endpoint = "$endpoint/api/mouse-companion/test-render-proof-sweep"
} else {
    $endpoint = "$endpoint/api/mouse-companion/test-render-proof"
}

$payload = [ordered]@{
    x = $X
    y = $Y
    button = $Button
    delta = $Delta
    hold_ms = $HoldMs
    wait_for_frame_ms = $WaitForFrameMs
    expect_frame_advance = $ExpectFrameAdvance
    expected_backend = $ExpectedBackend
    expect_preview_active = $ExpectPreviewActive
}
if ($Route -eq "proof") {
    $payload.event = $Event
}

$headers = @{
    "x-mfcmouseeffect-token" = $Token
    "Content-Type" = "application/json"
}

$payloadJson = $payload | ConvertTo-Json -Depth 8 -Compress

try {
    $response = Invoke-RestMethod -Method Post -Uri $endpoint -Headers $headers -Body $payloadJson
} catch {
    Fail "request failed: $($_.Exception.Message)"
}

if ($Route -eq "sweep") {
    foreach ($item in @($response.results)) {
        if ($null -ne $item) {
            Add-DefaultLaneSummaryProperty $item.real_renderer_preview
            if ($null -ne $item.proof) {
                Add-DefaultLaneSummaryProperty $item.proof.renderer_runtime_after
            }
        }
    }
} else {
    Add-DefaultLaneSummaryProperty $response.real_renderer_preview
    Add-DefaultLaneSummaryProperty $response.renderer_runtime_after
}

$responseJson = $response | ConvertTo-Json -Depth 16
if (-not [string]::IsNullOrWhiteSpace($JsonOutput)) {
    $responseJson | Set-Content -LiteralPath $JsonOutput -Encoding UTF8
    Write-Ok "saved proof json: $JsonOutput"
}

if ($Route -eq "sweep") {
    $summary = $response.summary
    $allExpectationsMet = [bool]$summary.all_expectations_met
    $appearanceFailures = New-Object System.Collections.Generic.List[string]
    $pluginFailures = New-Object System.Collections.Generic.List[string]
    if ($ExpectAppearanceProfileMatch) {
        foreach ($item in @($response.results)) {
            $afterCheck = Test-AppearanceExpectation $item.proof.renderer_runtime_after $expectedAppearance ("{0}:renderer_runtime_after" -f $item.event)
            foreach ($failure in @($afterCheck.mismatches)) {
                $appearanceFailures.Add([string]$failure)
            }
            $previewCheck = Test-AppearanceExpectation $item.real_renderer_preview $expectedAppearance ("{0}:real_renderer_preview" -f $item.event)
            foreach ($failure in @($previewCheck.mismatches)) {
                $appearanceFailures.Add([string]$failure)
            }
        }
    }
    if ((-not [string]::IsNullOrWhiteSpace($ExpectedAppearancePluginKind)) -or
        $ExpectAppearancePluginMetadataPresent -or
        (-not [string]::IsNullOrWhiteSpace($ExpectedAppearanceSemanticsMode)) -or
        (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneCandidate)) -or
        (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneSource)) -or
        (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneRolloutStatus)) -or
        (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneStyleIntent))) {
        foreach ($item in @($response.results)) {
            $runtimePluginCheck = Test-RendererPluginExpectation `
                $item.proof.renderer_runtime_after `
                $ExpectedAppearancePluginKind `
                $ExpectAppearancePluginMetadataPresent `
                $ExpectedAppearanceSemanticsMode `
                $ExpectedDefaultLaneCandidate `
                $ExpectedDefaultLaneSource `
                $ExpectedDefaultLaneRolloutStatus `
                $ExpectedDefaultLaneStyleIntent `
                ("{0}:renderer_runtime_after" -f $item.event)
            foreach ($failure in @($runtimePluginCheck.mismatches)) {
                $pluginFailures.Add([string]$failure)
            }
            $previewPluginCheck = Test-RendererPluginExpectation `
                $item.real_renderer_preview `
                $ExpectedAppearancePluginKind `
                $ExpectAppearancePluginMetadataPresent `
                $ExpectedAppearanceSemanticsMode `
                $ExpectedDefaultLaneCandidate `
                $ExpectedDefaultLaneSource `
                $ExpectedDefaultLaneRolloutStatus `
                $ExpectedDefaultLaneStyleIntent `
                ("{0}:real_renderer_preview" -f $item.event)
            foreach ($failure in @($previewPluginCheck.mismatches)) {
                $pluginFailures.Add([string]$failure)
            }
        }
    }
    $appearanceExpectationMet = ($appearanceFailures.Count -eq 0)
    $pluginExpectationMet = ($pluginFailures.Count -eq 0)
    $allExpectationsMet = $allExpectationsMet -and $appearanceExpectationMet -and $pluginExpectationMet
    if ($allExpectationsMet) {
        Write-Ok "render proof sweep"
    } else {
        Write-Host "[mfx:fail] render proof sweep"
    }
    Write-Host ("  - expectations={0}/{1} frame_advanced={2} backend_checks={3}/{4} preview_checks={5}/{6}" -f `
        $summary.expectation_met_count, `
        $summary.expectation_requested_count, `
        $summary.frame_advanced_count, `
        $summary.backend_expectation_met_count, `
        $summary.backend_expectation_count, `
        $summary.preview_expectation_met_count, `
        $summary.preview_expectation_count)
    if ($ExpectAppearanceProfileMatch) {
        Write-Host ("  - appearance_check={0}" -f $appearanceExpectationMet)
    }
    if ((-not [string]::IsNullOrWhiteSpace($ExpectedAppearancePluginKind)) -or
        $ExpectAppearancePluginMetadataPresent -or
        (-not [string]::IsNullOrWhiteSpace($ExpectedAppearanceSemanticsMode))) {
        Write-Host ("  - plugin_check={0}" -f $pluginExpectationMet)
    }
    foreach ($item in $response.results) {
        $proof = $item.proof
        $deltaNode = $proof.renderer_runtime_delta
        $preview = $item.real_renderer_preview
        Write-Host ("  - {0}: status={1} frame_delta={2} backend={3} preview_active={4} default_lane={5} preset={6}->{7} combo={8}" -f `
            $item.event, `
            $proof.renderer_runtime_expectation_status, `
            $deltaNode.frame_count_delta, `
            $item.selected_renderer_backend, `
            $preview.preview_active, `
            (Format-DefaultLaneSummary $preview), `
            $preview.appearance_requested_preset_id, `
            $preview.appearance_resolved_preset_id, `
            $preview.appearance_combo_preset)
    }
    foreach ($failure in @($appearanceFailures)) {
        Write-Host ("  - appearance_mismatch: {0}" -f $failure)
    }
    foreach ($failure in @($pluginFailures)) {
        Write-Host ("  - plugin_mismatch: {0}" -f $failure)
    }
    if (-not $allExpectationsMet) {
        exit 1
    }
    exit 0
}

$deltaNode = $response.renderer_runtime_delta
$frameExpectationMet = [bool]$response.renderer_runtime_expectation_met
$backendExpectationMet = [bool]$response.backend_expectation_met
$previewExpectationMet = [bool]$response.preview_expectation_met
$allExpectationsMet = [bool]$response.all_expectations_met
$appearanceExpectationMet = $true
$appearanceFailures = @()
$pluginExpectationMet = $true
$pluginFailures = @()
if ($ExpectAppearanceProfileMatch) {
    $runtimeAppearanceCheck =
        Test-AppearanceExpectation $response.renderer_runtime_after $expectedAppearance "renderer_runtime_after"
    $previewAppearanceCheck =
        Test-AppearanceExpectation $response.real_renderer_preview $expectedAppearance "real_renderer_preview"
    $appearanceFailures = @($runtimeAppearanceCheck.mismatches + $previewAppearanceCheck.mismatches)
    $appearanceExpectationMet = ($appearanceFailures.Count -eq 0)
    $allExpectationsMet = $allExpectationsMet -and $appearanceExpectationMet
}
if ((-not [string]::IsNullOrWhiteSpace($ExpectedAppearancePluginKind)) -or
    $ExpectAppearancePluginMetadataPresent -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedAppearanceSemanticsMode)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneCandidate)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneSource)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneRolloutStatus)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneStyleIntent))) {
    $runtimePluginCheck =
        Test-RendererPluginExpectation `
            $response.renderer_runtime_after `
            $ExpectedAppearancePluginKind `
            $ExpectAppearancePluginMetadataPresent `
            $ExpectedAppearanceSemanticsMode `
            $ExpectedDefaultLaneCandidate `
            $ExpectedDefaultLaneSource `
            $ExpectedDefaultLaneRolloutStatus `
            $ExpectedDefaultLaneStyleIntent `
            "renderer_runtime_after"
    $previewPluginCheck =
        Test-RendererPluginExpectation `
            $response.real_renderer_preview `
            $ExpectedAppearancePluginKind `
            $ExpectAppearancePluginMetadataPresent `
            $ExpectedAppearanceSemanticsMode `
            $ExpectedDefaultLaneCandidate `
            $ExpectedDefaultLaneSource `
            $ExpectedDefaultLaneRolloutStatus `
            $ExpectedDefaultLaneStyleIntent `
            "real_renderer_preview"
    $pluginFailures = @($runtimePluginCheck.mismatches + $previewPluginCheck.mismatches)
    $pluginExpectationMet = ($pluginFailures.Count -eq 0)
    $allExpectationsMet = $allExpectationsMet -and $pluginExpectationMet
}

if ($allExpectationsMet) {
    Write-Ok "render proof"
} else {
    Write-Host "[mfx:fail] render proof"
}
Write-Host ("  - status={0} frame_delta={1} backend={2} preview_active={3} frame_check={4} backend_check={5} preview_check={6} appearance_check={7}" -f `
    $response.renderer_runtime_expectation_status, `
    $deltaNode.frame_count_delta, `
    $response.selected_renderer_backend, `
    $response.real_renderer_preview.preview_active, `
    $frameExpectationMet, `
    $backendExpectationMet, `
    $previewExpectationMet, `
    $appearanceExpectationMet)
if ((-not [string]::IsNullOrWhiteSpace($ExpectedAppearancePluginKind)) -or
    $ExpectAppearancePluginMetadataPresent -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedAppearanceSemanticsMode)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneCandidate)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneSource)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneRolloutStatus)) -or
    (-not [string]::IsNullOrWhiteSpace($ExpectedDefaultLaneStyleIntent))) {
    Write-Host ("  - plugin kind={0} metadata_path={1} semantics_mode={2} default_lane={3}/{4}/{5}/{6} plugin_check={7}" -f `
        $response.real_renderer_preview.appearance_plugin_kind, `
        $response.real_renderer_preview.appearance_plugin_metadata_path, `
        $response.real_renderer_preview.appearance_plugin_appearance_semantics_mode, `
        $response.real_renderer_preview.default_lane_candidate, `
        $response.real_renderer_preview.default_lane_source, `
        $response.real_renderer_preview.default_lane_rollout_status, `
        $response.real_renderer_preview.default_lane_style_intent, `
        $pluginExpectationMet)
    Write-Host ("  - default_lane_summary={0}" -f `
        (Format-DefaultLaneSummary $response.real_renderer_preview))
}
Write-Host ("  - appearance preset={0}->{1} skin={2} accessory_family={3} combo={4}" -f `
    $response.real_renderer_preview.appearance_requested_preset_id, `
    $response.real_renderer_preview.appearance_resolved_preset_id, `
    $response.real_renderer_preview.appearance_skin_variant_id, `
    $response.real_renderer_preview.appearance_accessory_family, `
    $response.real_renderer_preview.appearance_combo_preset)
foreach ($failure in @($appearanceFailures)) {
    Write-Host ("  - appearance_mismatch: {0}" -f $failure)
}
foreach ($failure in @($pluginFailures)) {
    Write-Host ("  - plugin_mismatch: {0}" -f $failure)
}

if (-not $allExpectationsMet) {
    exit 1
}
