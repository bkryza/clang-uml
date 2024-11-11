param ($Prefix="C:\clang-uml-llvm18", $BuildType="Release")

$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

try {
  cmake -S . -B $BuildType -DCMAKE_PREFIX_PATH="$Prefix" -DENABLE_CXX_MODULES_TEST_CASES=OFF -Thost=x64
  cmake --build $BuildType --config $BuildType -- "-logger:$PWD/util/msbuild_compile_commands_logger/bin/Debug/netstandard2.0/CompileCommandsLogger.dll"

  # Create compile commands in Visual Studio
  # before running these tests
  cd $BuildType
  ctest -C $BuildType --output-on-failure
}
catch {
  break
}

cd ..
