Param(
    [String]$qmodname = "discord-presence"
)
# Builds a .qmod file for loading with QuestPatcher or BMBF
$NDKPath = Get-Content $PSScriptRoot/ndkpath.txt

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

$ArchiveName = "$qmodname.qmod"
$TempArchiveName = "$qmodname.qmod.zip"

& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk
Compress-Archive -Path "./libs/arm64-v8a/libdiscord-presence.so", "./libs/arm64-v8a/libbeatsaber-hook_3_14_0.so", "./mod.json", "./module.json", "./cover.png" -DestinationPath $TempArchiveName -Force
Move-Item $TempArchiveName $ArchiveName -Force