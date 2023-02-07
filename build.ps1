param ($Prefix="C:\clang-uml", $BuildType="Release")

cmake -S . -B .\_build\$BuildType -DCMAKE_PREFIX_PATH="$Prefix" -Thost=x64
cmake --build .\_build\$BuildType --config $BuildType