if (Test-Path "$PSScriptRoot/ndkpath.txt") {
    $NDKPath = Get-Content "$PSScriptRoot/ndkpath.txt"
} else {
    $NDKPath = $env:ANDROID_NDK_HOME
}

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

# & $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk

& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B ./build .
& cmake --build ./build

