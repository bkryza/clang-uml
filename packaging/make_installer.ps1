# This script assumes that all clang-uml dependencies are instaled in C:\clang-uml

param ($Prefix="C:\clang-uml-llvm18", $BuildType="Release")

mkdir _BUILD

cmake -S .. -B .\_BUILD\windows\ -DCMAKE_PREFIX_PATH="$Prefix" -Thost=x64

cd .\_BUILD\windows\src

msbuild .\clang-uml.vcxproj -maxcpucount /p:Configuration=Release

cd ..

cpack -C "Release" -G NSIS64 

cd ..
cd ..