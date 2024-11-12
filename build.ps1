param (
  $Prefix="C:\clang-uml-llvm19",
  $BuildType="Release"
)

# Save the original directory
$originalDirectory = Get-Location

$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

try {
  Set-Location util/msbuild_compile_commands_logger
  dotnet build
  Copy-Item bin/Debug/netstandard2.0/CompileCommandsLogger.dll CompileCommandsLogger.dll
  Set-Location $originalDirectory

  cmake -G "Visual Studio 17 2022" -S . -B $BuildType -DCMAKE_PREFIX_PATH="$Prefix" -Thost=x64

  cmake --build $BuildType --config $BuildType -- "-logger:$PWD/util/msbuild_compile_commands_logger/CompileCommandsLogger.dll"

  Set-Location $BuildType

  ctest -C $BuildType --output-on-failure
}
catch {
  Write-Host $Error[0]
  Write-Host "Exiting script."
  return
}
finally {
  # Always return to the original directory
  Set-Location $originalDirectory
}