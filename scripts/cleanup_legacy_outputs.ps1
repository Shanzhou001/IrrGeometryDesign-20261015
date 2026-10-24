param([string]$Report = 'results/refactor_20261024/cleanup.json')
$ErrorActionPreference = 'Stop'
$refactorRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot)).TrimEnd('\')
$reportPath = [IO.Path]::GetFullPath((Join-Path $refactorRoot $Report))
if (-not $reportPath.StartsWith($refactorRoot + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Report must stay in the workspace.' }
if (Test-Path -LiteralPath $reportPath) { throw 'Cleanup report already exists.' }
$retiredBuilds = @('IrrGeometryDesign', 'x64', 'tools')
$checkedTargets = @()
$removedFiles = @()
foreach ($name in $retiredBuilds) {
    $target = [IO.Path]::GetFullPath((Join-Path $refactorRoot $name))
    if (-not $target.StartsWith($refactorRoot + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Unsafe cleanup target.' }
    if (-not (Test-Path -LiteralPath $target)) { continue }
    foreach ($file in Get-ChildItem -LiteralPath $target -File -Recurse -Force) {
        $relative = $file.FullName.Substring($target.Length + 1)
        $allowed = if ($name -eq 'IrrGeometryDesign') {
            $relative.StartsWith('GeneratedFiles\') -or $relative.StartsWith('x64\')
        } elseif ($name -eq 'tools') {
            $relative.StartsWith('__pycache__\') -and $file.Extension -eq '.pyc'
        } else {
            @('.dll','.exe','.exp','.lib','.qm','.ilk','.pdb') -contains $file.Extension
        }
        if (-not $allowed) { throw ('Unexpected user/source file; refusing cleanup: ' + $file.FullName) }
        $removedFiles += [ordered]@{ path = $file.FullName.Substring($refactorRoot.Length + 1); bytes = $file.Length; sha256 = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
    }
    $checkedTargets += $target
}
$state = [ordered]@{ completed = $false; resolved_targets = $checkedTargets; generated_build_files = $removedFiles; root_logs = @() }
$state | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $reportPath -Encoding utf8
# All absolute targets and contents were checked above; use one shell for deletion.
foreach ($target in $checkedTargets) { Remove-Item -LiteralPath $target -Recurse -Force }
$logDirectory = Join-Path $refactorRoot 'results/refactor_20261024/legacy_root_logs'
foreach ($name in @('build_release_20261023.log', 'build_release_20261023_final.log')) {
    $source = Join-Path $refactorRoot $name
    if (-not (Test-Path -LiteralPath $source)) { continue }
    New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
    $destination = Join-Path $logDirectory $name
    if (Test-Path -LiteralPath $destination) { throw ('Existing archived log: ' + $destination) }
    $state.root_logs += [ordered]@{ path = $name; destination = $destination.Substring($refactorRoot.Length + 1); sha256 = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant() }
    Move-Item -LiteralPath $source -Destination $destination
}
$state.completed = $true
$state | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $reportPath -Encoding utf8
Write-Output ('Removed generated build files: ' + $removedFiles.Count + '; archived root logs: ' + $state.root_logs.Count)
