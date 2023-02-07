param ($Prefix="C:\clang-uml", $BuildType="Release")

cmake -S . -B $BuildType -DCMAKE_PREFIX_PATH="$Prefix" -Thost=x64
cmake --build $BuildType --config $BuildType

# Create compile commands in Visual Studio
# before running these tests
cd $BuildType
ctest -C $BuildType --output-on-failure
cd ..