[CmdletBinding()]
param(
    [string]$BaseUrl = "",
    [string]$Token = "",
    [string]$RuntimeFile = "",
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
  -Route <proof|sweep>         Route kind (default: sweep)
  -Preset <name>               Named preset (currently: real-preview-smoke)
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

function Resolve-DefaultRuntimeFile {
    if (-not [string]::IsNullOrWhiteSpace($env:MFX_WEBSETTINGS_RUNTIME_FILE)) {
        return $env:MFX_WEBSETTINGS_RUNTIME_FILE
    }
    if (-not [string]::IsNullOrWhiteSpace($env:APPDATA)) {
        return (Join-Path $env:APPDATA "MFCMouseEffect\websettings_runtime_auto.json")
    }
    return ""
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

if ($Help) {
    Show-Usage
    exit 0
}

$runtimeInfo = $null

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
    default {
        Fail "invalid -Preset value: $Preset (expected: real-preview-smoke)"
    }
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

$responseJson = $response | ConvertTo-Json -Depth 16
if (-not [string]::IsNullOrWhiteSpace($JsonOutput)) {
    $responseJson | Set-Content -LiteralPath $JsonOutput -Encoding UTF8
    Write-Ok "saved proof json: $JsonOutput"
}

if ($Route -eq "sweep") {
    $summary = $response.summary
    $allExpectationsMet = [bool]$summary.all_expectations_met
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
    foreach ($item in $response.results) {
        $proof = $item.proof
        $deltaNode = $proof.renderer_runtime_delta
        $preview = $item.real_renderer_preview
        Write-Host ("  - {0}: status={1} frame_delta={2} backend={3} preview_active={4}" -f `
            $item.event, `
            $proof.renderer_runtime_expectation_status, `
            $deltaNode.frame_count_delta, `
            $item.selected_renderer_backend, `
            $preview.preview_active)
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

if ($allExpectationsMet) {
    Write-Ok "render proof"
} else {
    Write-Host "[mfx:fail] render proof"
}
Write-Host ("  - status={0} frame_delta={1} backend={2} preview_active={3} frame_check={4} backend_check={5} preview_check={6}" -f `
    $response.renderer_runtime_expectation_status, `
    $deltaNode.frame_count_delta, `
    $response.selected_renderer_backend, `
    $response.real_renderer_preview.preview_active, `
    $frameExpectationMet, `
    $backendExpectationMet, `
    $previewExpectationMet)

if (-not $allExpectationsMet) {
    exit 1
}
