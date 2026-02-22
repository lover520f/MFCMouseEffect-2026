param(
    [string]$ProjectFile = "MFCMouseEffect/MFCMouseEffect.vcxproj"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if (-not (Test-Path $ProjectFile)) {
    Write-Error "Project file not found: $ProjectFile"
    exit 2
}

[xml]$xml = Get-Content $ProjectFile
$ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
$ns.AddNamespace("msb", "http://schemas.microsoft.com/developer/msbuild/2003")
$compileNodes = $xml.SelectNodes("//msb:ClCompile", $ns)

$root = (Resolve-Path ".").Path

$tokenRules = @(
    @{ Name = "windows.h"; Regex = "#include\s*<windows\.h>" },
    @{ Name = "d3d11.h"; Regex = "#include\s*<d3d11\.h>" },
    @{ Name = "d2d1.h"; Regex = "#include\s*<d2d1\.h>" },
    @{ Name = "dcomp.h"; Regex = "#include\s*<dcomp\.h>" },
    @{ Name = "dxgi*"; Regex = "#include\s*<dxgi" },
    @{ Name = "afx token"; Regex = "\bAfx[A-Za-z0-9_]+\b" },
    @{ Name = "CWnd token"; Regex = "\bCWnd\b" }
)

$violations = @()

foreach ($node in $compileNodes) {
    $includePath = $node.GetAttribute("Include")
    if ([string]::IsNullOrWhiteSpace($includePath)) { continue }

    # Platform code is allowed to include platform-specific dependencies.
    if ($includePath.StartsWith("Platform\")) { continue }

    $fullPath = Join-Path $root ("MFCMouseEffect/" + $includePath.Replace("\", "/"))
    if (-not (Test-Path $fullPath)) { continue }

    $content = Get-Content $fullPath -Raw
    foreach ($rule in $tokenRules) {
        if ($content -match $rule.Regex) {
            $violations += [PSCustomObject]@{
                File  = $includePath
                Token = $rule.Name
            }
        }
    }
}

if ($violations.Count -eq 0) {
    Write-Output "OK: no direct Win32/MFC boundary violations in non-Platform compile units."
    exit 0
}

Write-Output "Found boundary violations in non-Platform compile units:"
$violations |
    Sort-Object File, Token -Unique |
    ForEach-Object { Write-Output ("- {0} [{1}]" -f $_.File, $_.Token) }

exit 1
