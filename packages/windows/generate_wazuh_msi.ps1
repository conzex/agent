param (
    [string]$MSI_NAME = "defendx-agent",
    [string]$SIGN = "no",
    [string]$DEBUG = "no",
    [string]$CMAKE_CONFIG = "Debug",
    [string]$SIGN_TOOLS_PATH = "",
    [string]$CV2PDB_PATH = "",
    [string]$CERTIFICATE_PATH = "",
    [string]$CERTIFICATE_PASSWORD = "",
    [switch]$help
)

$SIGNTOOL_EXE = "signtool.exe"
$CV2PDB_EXE = "cv2pdb.exe"

if ($help) {
    "
    This tool can be used to generate the Windows DefendX Agent MSI package.

    PARAMETERS TO BUILD DEFENDX-AGENT MSI (OPTIONAL):
        1. MSI_NAME: MSI package name output. Default: 'defendx-agent'.
        2. SIGN: Define signing action (yes or no). Default: 'no'.
        3. DEBUG: Generate debug symbols (yes or no). Default: 'no'.
        4. CMAKE_CONFIG: CMake config type (Debug, Release, etc.). Default: 'Debug'.
        5. SIGN_TOOLS_PATH: Path for signing tools (if not set in PATH env).
        6. CV2PDB_PATH: Debug symbols tools path (if not set in PATH env).
        7. CERTIFICATE_PATH: Path to .pfx certificate file for signing.
        8. CERTIFICATE_PASSWORD: Password for the .pfx certificate file.

    USAGE:
        * DEFENDX:
          $ ./generate_defendx_msi.ps1 -MSI_NAME defendx-agent -SIGN yes -DEBUG no
    "
    Exit
}

function BuildDefendXMsi() {
    $MSI_NAME = "$MSI_NAME.msi"
    Write-Host "MSI_NAME = $MSI_NAME"

    if ($DEBUG -eq "yes") {
        if ($CV2PDB_PATH -ne "") {
            $CV2PDB_EXE = "$CV2PDB_PATH/$CV2PDB_EXE"
        }

        $originalDir = Get-Location
        cd "$PSScriptRoot/../../build/$CMAKE_CONFIG"

        $exeFiles = Get-ChildItem -Filter "*.exe"
        $exeFiles += Get-ChildItem -Filter "*.dll"

        foreach ($file in $exeFiles) {
            Write-Host "Extracting debug symbols from $($file.FullName)"
            $args = "$($file.FullName) $($file.FullName) $($file.BaseName).pdb"
            Start-Process -FilePath $CV2PDB_EXE -ArgumentList $args -WindowStyle Hidden
        }

        Write-Host "Waiting for processes to finish"
        Wait-Process -Name cv2pdb -Timeout 10

        $pdbFiles = Get-ChildItem -Filter "*.pdb"
        $ZIP_NAME = "$($MSI_NAME.Replace('.msi', '-debug-symbols.zip'))"

        Write-Host "Compressing debug symbols to $ZIP_NAME"
        Compress-Archive -Path $pdbFiles -Force -DestinationPath "$ZIP_NAME"

        Remove-Item -Path "*.pdb"
        cd $originalDir
    }

    if ($SIGN_TOOLS_PATH -ne "") {
        $SIGNTOOL_EXE = "$SIGN_TOOLS_PATH/$SIGNTOOL_EXE"
    }

    if ($SIGN -eq "yes") {
        $signOptions = @()
        if ($CERTIFICATE_PATH -ne "" -and $CERTIFICATE_PASSWORD -ne "") {
            $signOptions += "/f `"$CERTIFICATE_PATH`" /p `"$CERTIFICATE_PASSWORD`""
        } else {
            $signOptions += "/a"
        }

        $filesToSign = @(
            "$PSScriptRoot\..\..\build\$CMAKE_CONFIG\*.exe",
            "$PSScriptRoot\..\..\build\$CMAKE_CONFIG\*.dll",
            "$PSScriptRoot\postinstall.ps1",
            "$PSScriptRoot\cleanup.ps1"
        )

        foreach ($file in $filesToSign) {
            Write-Host "Signing $file"
            & $SIGNTOOL_EXE sign $signOptions /tr http://timestamp.digicert.com /fd SHA256 /td SHA256 "$file"
        }
    }

    Write-Host "Building MSI installer..."
    cpack -C $CMAKE_CONFIG --config "$PSScriptRoot\..\..\build\CPackConfig.cmake" -B "$PSScriptRoot"

    if ($SIGN -eq "yes") {
        Write-Host "Signing $MSI_NAME..."
        & $SIGNTOOL_EXE sign $signOptions /tr http://timestamp.digicert.com /d $MSI_NAME /fd SHA256 /td SHA256 "$PSScriptRoot\$MSI_NAME"
    }
}

BuildDefendXMsi
