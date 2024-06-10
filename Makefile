# Makefile
#
# Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This Makefile is just a handy wrapper around cmake
#

.DEFAULT_GOAL := debug

OS_UNAME := $(shell uname -s)

ifeq ($(OS_UNAME),Linux)
	NUMPROC ?= $(shell nproc)
else ifeq ($(OS_UNAME),Darwin)
	NUMPROC ?= $(shell sysctl -n hw.logicalcpu)
else
	NUMPROC ?= 1
endif

LLVM_VERSION ?=
LLVM_CONFIG_PATH ?=
LLVM_SHARED ?= ON
CMAKE_PREFIX ?=
CMAKE_CXX_FLAGS ?=
CMAKE_EXE_LINKER_FLAGS ?=
CMAKE_GENERATOR ?= Unix Makefiles
CODE_COVERAGE ?= OFF

ENABLE_CXX_MODULES_TEST_CASES ?= OFF
ENABLE_CUDA_TEST_CASES ?= OFF

GIT_VERSION	?= $(shell git describe --tags --always --abbrev=7)
PKG_VERSION	?= $(shell git describe --tags --always --abbrev=7 | tr - .)
GIT_COMMIT ?= $(shell git rev-parse HEAD)
GIT_BRANCH ?= $(shell git rev-parse --abbrev-ref HEAD)

DESTDIR ?=

.PHONY: clean
clean:
	rm -rf debug release debug_tidy coverage.info coverage-src.info

debug/CMakeLists.txt:
	cmake -S . -B debug \
		-G"$(CMAKE_GENERATOR)" \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_VERSION=${LLVM_VERSION} \
		-DLLVM_CONFIG_PATH=${LLVM_CONFIG_PATH} \
		-DLINK_LLVM_SHARED=${LLVM_SHARED} \
		-DCMAKE_PREFIX=${CMAKE_PREFIX} \
		-DENABLE_CUDA_TEST_CASES=$(ENABLE_CUDA_TEST_CASES) \
		-DENABLE_CXX_MODULES_TEST_CASES=$(ENABLE_CXX_MODULES_TEST_CASES) \
		-DCODE_COVERAGE=$(CODE_COVERAGE)

release/CMakeLists.txt:
	cmake -S . -B release \
		-G"$(CMAKE_GENERATOR)" \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_VERSION=${LLVM_VERSION} \
		-DLLVM_CONFIG_PATH=${LLVM_CONFIG_PATH} \
		-DLINK_LLVM_SHARED=${LLVM_SHARED} \
		-DCMAKE_PREFIX=${CMAKE_PREFIX} \
		-DENABLE_CUDA_TEST_CASES=$(ENABLE_CUDA_TEST_CASES) \
		-DENABLE_CXX_MODULES_TEST_CASES=$(ENABLE_CXX_MODULES_TEST_CASES)

debug_tidy/CMakeLists.txt:
	cmake -S . -B debug_tidy \
		-G"$(CMAKE_GENERATOR)" \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_TESTS=OFF \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_VERSION=${LLVM_VERSION} \
		-DLLVM_CONFIG_PATH=${LLVM_CONFIG_PATH} \
		-DLINK_LLVM_SHARED=${LLVM_SHARED} \
		-DCMAKE_PREFIX=${CMAKE_PREFIX} \
		-DENABLE_CUDA_TEST_CASES=$(ENABLE_CUDA_TEST_CASES) \
		-DENABLE_CXX_MODULES_TEST_CASES=$(ENABLE_CXX_MODULES_TEST_CASES)

debug: debug/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	cmake --build debug -j$(NUMPROC)

debug_tidy: debug_tidy/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	cmake --build debug_tidy -j$(NUMPROC)

release: release/CMakeLists.txt
	cmake --build release -j$(NUMPROC)

test: debug
	CTEST_OUTPUT_ON_FAILURE=1 ctest --test-dir debug

test_release: release
	CTEST_OUTPUT_ON_FAILURE=1 ctest --test-dir release

coverage_report: test
	lcov -c -d debug -o coverage.info
	lcov -r coverage.info -o coverage-src.info "${PWD}/src/main.cc" "${PWD}/src/common/generators/generators.cc"
	lcov -e coverage-src.info -o coverage-src.info "${PWD}/src/*"
	lcov -l coverage-src.info
	genhtml coverage-src.info --output-directory debug/coverage_html

install: release
	make -C release install DESTDIR=${DESTDIR}

test_diagrams: test
	mkdir -p debug/tests/diagrams/plantuml
	mkdir -p debug/tests/diagrams/mermaid
	plantuml -tsvg -nometadata -o plantuml debug/tests/diagrams/*.puml
	python3 util/validate_json.py debug/tests/diagrams/*.json
	python3 util/generate_mermaid.py debug/tests/diagrams/*.mmd

document_test_cases: test_diagrams
	python3 util/generate_test_cases_docs.py
	# Format generated SVG files
	python3 util/format_svg.py docs/test_cases/*.svg

clanguml_diagrams: debug
	mkdir -p docs/diagrams/plantuml
	mkdir -p docs/diagrams/mermaid
	debug/src/clang-uml -g plantuml -g json -g mermaid -p
	# Convert .puml files to svg images
	plantuml -tsvg -nometadata -o plantuml docs/diagrams/*.puml
	# Convert .mmd files to svg images
	python3 util/generate_mermaid.py docs/diagrams/*.mmd
	# Format generated SVG files
	python3 util/format_svg.py docs/diagrams/plantuml/*.svg
	python3 util/format_svg.py docs/diagrams/mermaid/*.svg

.PHONY: submodules
submodules:
	git submodule update --init --recursive

.PHONY: init_compile_commands
init_compile_commands: debug
	ln -sf debug/compile_commands.json compile_commands.json

.PHONY: init
	init: submodules init_compile_commands

.PHONY: clang-format
clang-format:
	docker run --rm -v $(CURDIR):/root/sources bkryza/clang-format-check:1.4

.PHONY: format
format:
	docker run --rm -v $(CURDIR):/root/sources bkryza/clang-format-check:1.4

.PHONY: debug_tidy
tidy: debug_tidy
	run-clang-tidy-17 -extra-arg=-Wno-unknown-warning-option -j $(NUMPROC) -p debug_tidy ./src

.PHONY: check-formatting
check-formatting:
	./util/check_formatting.sh

.PHONY: iwyu_fixes
iwyu_fixes: debug
	python3 $(shell which iwyu_tool.py) -p debug > debug/iwyu.out
	python3 $(shell which fix_includes.py) -h --re_only="${PWD}/src/.*" < debug/iwyu.out
	python3 $(shell which fix_includes.py) -h --re_only="${PWD}/tests/.*" < debug/iwyu.out

.PHONY: docs
docs:
	make -C docs toc

.PHONY: doxygen
doxygen: docs
	cp CONTRIBUTING.md docs/contributing.md
	cp CHANGELOG.md docs/changelog.md
	mkdir -p docs/doxygen/html/test_cases
	cp docs/diagrams/plantuml/*.svg docs/doxygen/html/
	cp docs/test_cases/*.svg docs/doxygen/html/test_cases/
	../doxygen/_build/bin/doxygen

.PHONY: fedora/%
fedora/%:
	mkdir -p packaging/_BUILD/fedora/$*
	git archive --format=tar.gz --prefix=clang-uml-$(PKG_VERSION)/ v$(GIT_VERSION) >packaging/_BUILD/fedora/$*/clang-uml-$(PKG_VERSION).tar.gz
	docker run --cpuset-cpus=0-7 -v $(PWD):$(PWD) fedora:$* sh -c "dnf install -y make git && cd ${PWD} && make OS=fedora DIST=$* VERSION=${PKG_VERSION} COMMIT=${GIT_COMMIT} BRANCH=${GIT_BRANCH} -C packaging rpm"
