#!/bin/bash

set -e
trap 'echo "Build failed!"' ERR

declare -a llvm_configs=("llvm-config-12"
                         "llvm-config-13"
                         "llvm-config-14"
                         "llvm-config-15"
                         "llvm-config-16"
                         "llvm-config-17"
                         "llvm-config-18")

# Test with GCC and different LLVM versions
for config_path in ${llvm_configs[@]}; do
  echo "---------------------------------------------------------"
  echo " Running clang-uml tests against LLVM $(${config_path} --version)"
  echo "---------------------------------------------------------"
  make clean
  CC=/usr/bin/gcc-11 CXX=/usr/bin/g++-11 LLVM_CONFIG_PATH=$config_path NUMPROC=16 make test
done

# Also check compilation with Clang
make clean
CC=/usr/bin/clang-17 CXX=/usr/bin/clang++-17 LLVM_VERSION=17 NUMPROC=16 CMAKE_GENERATOR=Ninja ENABLE_CXX_MODULES_TEST_CASES=ON ENABLE_CUDA_TEST_CASES=ON make test
make clean
CC=/usr/bin/clang-18 CXX=/usr/bin/clang++-18 LLVM_VERSION=18 NUMPROC=16 CMAKE_GENERATOR=Ninja ENABLE_CXX_MODULES_TEST_CASES=ON ENABLE_CUDA_TEST_CASES=ON make test
make clean