Param(
    [String]$qmodname = "discord-presence"
)
# Builds a .qmod file for loading with QuestPatcher or BMBF
if (Test-Path "$PSScriptRoot/ndkpath.txt") {
    $NDKPath = Get-Content "$PSScriptRoot/ndkpath.txt"
} else {
    $NDKPath = $env:ANDROID_NDK_HOME
}

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

$ArchiveName = "$qmodname.qmod"
$TempArchiveName = "$qmodname.qmod.zip"

& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B ./build .
& cmake --build ./build

Compress-Archive -Path "./build/libdiscord-presence.so", "./build/libbeatsaber-hook_5_1_9.so","./build/libcustom-types.so","./build/libpaperlog.so","./build/libsl2.so", "./mod.json", "./module.json", "./cover.png" -DestinationPath $TempArchiveName -Force
Move-Item $TempArchiveName $ArchiveName -Force