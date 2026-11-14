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
        $DependencyDirectory = Join-Path $buildPath 'vcpkg_installed/x64-windows-static-custom'
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
if ($Static) {
    # A developer machine can hide accidental shared linkage through its installed
    # runtimes. Reject non-Windows imports before testing the EXE in isolation.
    $dumpbin = (Get-Command dumpbin.exe -ErrorAction Stop).Source
    $imports = & $dumpbin /NOLOGO /DEPENDENTS $executable
    if ($LASTEXITCODE) { throw "dumpbin failed with exit code $LASTEXITCODE" }
    $windowsDlls = @(
        'advapi32.dll', 'authz.dll', 'bcrypt.dll', 'bcryptprimitives.dll',
        'cfgmgr32.dll', 'comctl32.dll', 'comdlg32.dll', 'crypt32.dll',
        'd2d1.dll', 'd3d9.dll', 'd3d11.dll', 'd3dcompiler_47.dll', 'dcomp.dll',
        'dnsapi.dll', 'dwmapi.dll', 'dwrite.dll', 'dxgi.dll', 'dxva2.dll',
        'gdi32.dll', 'hid.dll', 'imm32.dll', 'iphlpapi.dll', 'kernel32.dll',
        'mpr.dll', 'msimg32.dll', 'mswsock.dll', 'netapi32.dll', 'normaliz.dll',
        'ntdll.dll', 'ole32.dll', 'oleacc.dll', 'oleaut32.dll', 'opengl32.dll',
        'powrprof.dll', 'propsys.dll', 'psapi.dll', 'rpcrt4.dll', 'secur32.dll',
        'setupapi.dll', 'shcore.dll', 'shell32.dll', 'shlwapi.dll', 'user32.dll',
        'userenv.dll', 'usp10.dll', 'uxtheme.dll', 'version.dll', 'winhttp.dll',
        'wininet.dll', 'winmm.dll', 'winspool.drv', 'wintrust.dll', 'wldap32.dll',
        'ws2_32.dll', 'wtsapi32.dll'
    )
    $dependencies = @($imports | ForEach-Object {
        if ($_ -match '^\s+([\w.+-]+\.(?:dll|drv))\s*$') { $Matches[1].ToLowerInvariant() }
    } | Sort-Object -Unique)
    if ($dependencies.Count -eq 0) { throw 'Could not read executable DLL dependencies.' }
    $externalDlls = @($dependencies | Where-Object {
        $_ -notin $windowsDlls -and $_ -notmatch '^(api|ext)-ms-win-(?!crt-)[\w-]+\.dll$'
    })
    if ($externalDlls.Count) {
        throw "Static frontend requires non-system DLLs: $($externalDlls -join ', ')"
    }
}
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
    @'
[Paths]
Prefix=.
Plugins=.
'@ | Set-Content -LiteralPath (Join-Path $packagePath 'qt.conf') -Encoding ascii
    Set-Content -LiteralPath (Join-Path $packagePath 'VERSION.txt') -Value $Version -Encoding ascii
    $packageInstructions = @"
InvokeInvoiceSystem $Version (Windows x64)
Extract the entire ZIP, then run InvokeInvoiceSystem.exe from this folder.
Keep the DLLs and plugin folders beside the executable.
The frontend connects to the configured Heroku backend by default.
"@
    Set-Content -LiteralPath (Join-Path $packagePath 'README.txt') -Encoding ascii -Value $packageInstructions
}
# Include the dependency copyright notices supplied by vcpkg.
$copyrightFiles = @(Get-ChildItem -Path (Join-Path $dependencyPath 'share/*/copyright') -File | Sort-Object FullName)
if ($Static) {
    # Notices are a separate release asset; the application folder contains only the EXE.
    if ($copyrightFiles.Count -eq 0) { throw 'No dependency copyright notices found.' }
    $noticesPath = Join-Path $outputPath "$packageName-licenses.txt"
    Set-Content -LiteralPath $noticesPath -Encoding utf8 -Value "InvokeInvoiceSystem $Version - dependency notices"
    foreach ($notice in $copyrightFiles) {
        Add-Content -LiteralPath $noticesPath -Encoding utf8 -Value "`n===== $($notice.Directory.Name) =====`n"
        Add-Content -LiteralPath $noticesPath -Encoding utf8 -Value (Get-Content -LiteralPath $notice.FullName -Raw -Encoding utf8)
    }
} else {
    $licenses = Join-Path $packagePath 'licenses'
    New-Item -ItemType Directory $licenses | Out-Null
    $copyrightFiles | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $licenses ($_.Directory.Name + '.txt'))
    }
}

# Check the actual package without developer DLL/plugin directories on PATH.
$savedPath = $env:PATH
$savedPluginPath = $env:QT_PLUGIN_PATH
$savedPlatformPluginPath = $env:QT_QPA_PLATFORM_PLUGIN_PATH
$savedPlatform = $env:QT_QPA_PLATFORM
$savedConsole = $env:QT_ASSUME_STDERR_HAS_CONSOLE
try {
    $env:PATH = "$env:SystemRoot/System32;$env:SystemRoot"
    $env:QT_PLUGIN_PATH = $packagePath
    $env:QT_QPA_PLATFORM_PLUGIN_PATH = $packagePath
    $env:QT_QPA_PLATFORM = 'windows'
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
    $env:QT_QPA_PLATFORM_PLUGIN_PATH = $savedPlatformPluginPath
    $env:QT_QPA_PLATFORM = $savedPlatform
    $env:QT_ASSUME_STDERR_HAS_CONSOLE = $savedConsole
}

if ($Static) {
    $asset = Join-Path $outputPath "$packageName.exe"
    Copy-Item -LiteralPath $executable -Destination $asset
} else {
    $asset = Join-Path $outputPath "$packageName.zip"
    Compress-Archive -LiteralPath $packagePath -DestinationPath $asset
}
$checksum = (Get-FileHash -LiteralPath $asset -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath "$asset.sha256" -Value "$checksum  $(Split-Path -Leaf $asset)" -Encoding ascii
Write-Output "Packaged and verified $asset"
