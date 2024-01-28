param ($Prefix="C:\clang-uml-llvm17", $BuildType="Release")

cmake -S . -B $BuildType -DCMAKE_PREFIX_PATH="$Prefix" -DENABLE_CXX_MODULES_TEST_CASES=OFF -Thost=x64
cmake --build $BuildType --config $BuildType

# Create compile commands in Visual Studio
# before running these tests
cd $BuildType
ctest -C $BuildType --output-on-failure
cd ..