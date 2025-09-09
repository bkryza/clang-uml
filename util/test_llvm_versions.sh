#!/bin/bash

set -e
trap 'echo "Build failed!"' ERR

if [ $# -ne 1 ]; then
    echo "Usage: $0 <llvm_version>"
    echo "Example: $0 18"
    exit 1
fi

LLVM_VERSION=$1

declare -a llvm_configs=("llvm-config-12"
                         "llvm-config-13"
                         "llvm-config-14"
                         "llvm-config-15"
                         "llvm-config-16"
                         "llvm-config-17"
                         "llvm-config-18"
                         "llvm-config-19"
                         "llvm-config-20"
                         "llvm-config-21")

# Check if the requested version is supported
config_path="llvm-config-${LLVM_VERSION}"
if [[ ! " ${llvm_configs[@]} " =~ " ${config_path} " ]]; then
    echo "Error: LLVM version ${LLVM_VERSION} is not supported."
    echo "Supported versions: 12, 13, 14, 15, 16, 17, 18, 19, 20, 21"
    exit 1
fi

# Test with GCC and the specified LLVM version
echo "---------------------------------------------------------"
echo " Running clang-uml tests against LLVM $(${config_path} --version)"
echo "---------------------------------------------------------"
make clean
CC=/usr/bin/gcc-13 CXX=/usr/bin/g++-13 LLVM_CONFIG_PATH=$config_path NUMPROC=16 make test

# Also check compilation with Clang if version >= 17
if [ "$LLVM_VERSION" -ge 17 ]; then
    make clean
    CC=/usr/bin/clang-${LLVM_VERSION} CXX=/usr/bin/clang++-${LLVM_VERSION} LLVM_VERSION=${LLVM_VERSION} \
    NUMPROC=16 \
    CMAKE_GENERATOR=Ninja \
    ENABLE_CXX_MODULES_TEST_CASES=ON \
    ENABLE_CUDA_TEST_CASES=ON \
    ENABLE_CUDA_TEST_CASES=ON \
    CLANG_UML_ENABLE_BACKTRACE=ON \
    ENABLE_OBJECTIVE_C_TEST_CASES=ON \
    FETCH_LIBOBJC2=OFF \
    ENABLE_BENCHMARKS=ON
    make test
fi

make clean
