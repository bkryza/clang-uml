#!/bin/bash

mkdir build && cd build

export PKG_CONFIG_PATH="$BUILD_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"

export CLANGUML_GIT_TOPLEVEL_DIR=${SRC_DIR}

cmake -DCMAKE_BUILD_TYPE=Release \
	  -DGIT_VERSION=${GIT_VERSION} \
	  -DCODE_COVERAGE=OFF \
	  -DBUILD_TESTS=OFF \
	  -DCMAKE_CXX_FLAGS="-Wno-nonnull -Wno-stringop-overflow -Wno-dangling-reference" \
	  -DLLVM_CONFIG_PATH=${BUILD_PREFIX}/bin/llvm-config \
	  -DCONDA_BUILD_PREFIX=${BUILD_PREFIX} \
	  -DCMAKE_INSTALL_PREFIX=${PREFIX} \
	  -DCMAKE_EXE_LINKER_FLAGS="-lyaml-cpp" \
	  ..

CTEST_OUTPUT_ON_FAILURE=1 make -j${CPU_COUNT}

make install