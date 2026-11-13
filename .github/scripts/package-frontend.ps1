param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9]+\.[0-9]+\.[0-9]+$')]
    [string]$Version,
    [string]$BuildDirectory = 'out/build/x64-release',
    [string]$OutputDirectory = 'out/release',
    [string]$DependencyDirectory = '',
    [switch]$Static
)

$ErrorActionPreference = 'Stop'
$buildPath = (Resolve-Path -LiteralPath $BuildDirectory).Path
if (-not $DependencyDirectory) {
    if ($Static) {
        $DependencyDirectory = Join-Path $PSScriptRoot '../../vcpkg_installed/x64-windows-static-custom'
    } else {
        $DependencyDirectory = Join-Path $buildPath 'vcpkg_installed/x64-windows'
    }
}
$dependencyPath = (Resolve-Path -LiteralPath $DependencyDirectory).Path
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
$outputPath = (Resolve-Path -LiteralPath $OutputDirectory).Path
$packageName = "InvokeInvoiceSystem-$Version-windows-x64"
$packagePath = Join-Path $outputPath $packageName
if (Test-Path -LiteralPath $packagePath) {
    throw "Package directory already exists: $packagePath. Use a fresh output directory."
}
New-Item -ItemType Directory $packagePath | Out-Null
Copy-Item -LiteralPath (Join-Path $buildPath 'InvokeInvoiceSystem.exe') -Destination $packagePath
$executable = Join-Path $packagePath 'InvokeInvoiceSystem.exe'
if (-not $Static) {
    # vcpkg app-local deployment supplies non-Qt libraries (MongoDB, OpenSSL, etc.).
    Get-ChildItem -LiteralPath $buildPath -Filter '*.dll' -File | Copy-Item -Destination $packagePath
    $deployQt = Join-Path $dependencyPath 'tools/Qt6/bin/windeployqt.exe'
    try {
        # Windows PowerShell 5 treats native stderr warnings as errors when redirected.
        # Use the tool's exit code to decide whether deployment succeeded.
        $ErrorActionPreference = 'Continue'
        & $deployQt --release --no-translations --compiler-runtime $executable
        $deployExitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = 'Stop'
    }
    if ($deployExitCode) { throw "windeployqt failed with exit code $deployExitCode" }

    # Include the app-local MSVC runtime so extracting the ZIP is sufficient.
    if (-not $env:VCToolsRedistDir) { throw 'Run packaging from an MSVC developer shell.' }
    $crtDirectories = @(Get-ChildItem -LiteralPath (Join-Path $env:VCToolsRedistDir 'x64') -Directory -Filter 'Microsoft.VC*.CRT')
    if ($crtDirectories.Count -ne 1) { throw 'Could not identify the x64 MSVC runtime directory.' }
    Get-ChildItem -LiteralPath $crtDirectories[0].FullName -Filter '*.dll' -File | Copy-Item -Destination $packagePath -Force
}
@'
[Paths]
Prefix=.
Plugins=.
'@ | Set-Content -LiteralPath (Join-Path $packagePath 'qt.conf') -Encoding ascii
Set-Content -LiteralPath (Join-Path $packagePath 'VERSION.txt') -Value $Version -Encoding ascii
if ($Static) {
    $packageInstructions = @"
InvokeInvoiceSystem $Version (Windows x64)
Run InvokeInvoiceSystem.exe directly. This package contains the self-contained frontend executable.
"@
} else {
    $packageInstructions = @"
InvokeInvoiceSystem $Version (Windows x64)
Extract the entire ZIP, then run InvokeInvoiceSystem.exe from this folder.
Keep the DLLs and plugin folders beside the executable.
The frontend connects to the configured Heroku backend by default.
"@
}
Set-Content -LiteralPath (Join-Path $packagePath 'README.txt') -Encoding ascii -Value $packageInstructions
# Include the dependency copyright notices supplied by vcpkg.
$licenses = Join-Path $packagePath 'licenses'
New-Item -ItemType Directory $licenses | Out-Null
Get-ChildItem -Path (Join-Path $dependencyPath 'share/*/copyright') -File | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $licenses ($_.Directory.Name + '.txt'))
}

# Check the actual package without developer DLL/plugin directories on PATH.
$savedPath = $env:PATH
$savedPluginPath = $env:QT_PLUGIN_PATH
$savedConsole = $env:QT_ASSUME_STDERR_HAS_CONSOLE
try {
    $env:PATH = "$env:SystemRoot/System32;$env:SystemRoot"
    $env:QT_PLUGIN_PATH = $packagePath
    $env:QT_ASSUME_STDERR_HAS_CONSOLE = '1'
    $stdoutPath = Join-Path $outputPath 'version-stdout.txt'
    $stderrPath = Join-Path $outputPath 'version-stderr.txt'
    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $executable
    $startInfo.Arguments = '--version'
    $startInfo.WorkingDirectory = $packagePath
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Hidden
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo
    try {
        if (-not $process.Start()) { throw 'Could not start the packaged frontend.' }
        $stdoutTask = $process.StandardOutput.ReadToEndAsync()
        $stderrTask = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(20000)) {
            $process.Kill()
            $process.WaitForExit()
            throw 'Packaged frontend did not finish its version check.'
        }
        $reportedVersion = $stdoutTask.GetAwaiter().GetResult().Trim()
        $stderr = $stderrTask.GetAwaiter().GetResult()
        Set-Content -LiteralPath $stdoutPath -Value $reportedVersion
        Set-Content -LiteralPath $stderrPath -Value $stderr
        if ($process.ExitCode -ne 0) { throw "Packaged frontend failed ($($process.ExitCode)): $stderr" }
    } finally {
        $process.Dispose()
    }
    if ($reportedVersion -ne "InvokeInvoiceSystem $Version") {
        throw "Unexpected packaged version: $reportedVersion"
    }
} finally {
    $env:PATH = $savedPath
    $env:QT_PLUGIN_PATH = $savedPluginPath
    $env:QT_ASSUME_STDERR_HAS_CONSOLE = $savedConsole
}

$archive = Join-Path $outputPath "$packageName.zip"
Compress-Archive -LiteralPath $packagePath -DestinationPath $archive
$checksum = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath "$archive.sha256" -Value "$checksum  $packageName.zip" -Encoding ascii
Write-Output "Packaged and verified $archive"
