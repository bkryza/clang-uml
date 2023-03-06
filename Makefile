# Makefile
#
# Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
CMAKE_CXX_FLAGS ?=
CMAKE_EXE_LINKER_FLAGS ?=

GIT_VERSION	?= $(shell git describe --tags --always --abbrev=7)
PKG_VERSION	?= $(shell git describe --tags --always --abbrev=7 | tr - .)
GIT_COMMIT ?= $(shell git rev-parse HEAD)
GIT_BRANCH ?= $(shell git rev-parse --abbrev-ref HEAD)

.PHONY: clean
clean:
	rm -rf debug release debug_tidy

debug/CMakeLists.txt:
	cmake -S . -B debug \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_VERSION=${LLVM_VERSION}

release/CMakeLists.txt:
	cmake -S . -B release \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_VERSION=${LLVM_VERSION}

debug_tidy/CMakeLists.txt:
	cmake -S . -B debug_tidy \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_TESTS=OFF \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_VERSION=${LLVM_VERSION}

debug: debug/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	make -C debug -j$(NUMPROC)

debug_tidy: debug_tidy/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	make -C debug_tidy -j$(NUMPROC)

release: release/CMakeLists.txt
	make -C release -j$(NUMPROC)

test: debug
	CTEST_OUTPUT_ON_FAILURE=1 make -C debug test

test_release: release
	CTEST_OUTPUT_ON_FAILURE=1 make -C release test

test_plantuml: test
	plantuml -tsvg debug/tests/puml/*.puml

document_test_cases: test_plantuml
	python3 util/generate_test_cases_docs.py
	python3 util/format_svg.py docs/test_cases/*.svg

clanguml_diagrams: debug
	mkdir -p docs/diagrams
	debug/src/clang-uml
	plantuml -tsvg -nometadata docs/diagrams/*.puml
	python3 util/format_svg.py docs/diagrams/*.svg

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
	run-clang-tidy-15 -j $(NUMPROC) -p debug_tidy ./src

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

.PHONY: fedora_36
fedora_36:
	mkdir -p packaging/_BUILD/fedora/36
	git archive --format=tar.gz --prefix=clang-uml-$(PKG_VERSION)/ v$(GIT_VERSION) >packaging/_BUILD/fedora/36/clang-uml-$(PKG_VERSION).tar.gz
	docker run --cpuset-cpus=0-7 -v $(PWD):$(PWD) fedora:36 sh -c "dnf install -y make git && cd ${PWD} && make OS=fedora DIST=36 VERSION=${PKG_VERSION} COMMIT=${GIT_COMMIT} BRANCH=${GIT_BRANCH} -C packaging rpm"

.PHONY: fedora_37
fedora_37:
	mkdir -p packaging/_BUILD/fedora/37
	git archive --format=tar.gz --prefix=clang-uml-$(PKG_VERSION)/ v$(GIT_VERSION) >packaging/_BUILD/fedora/37/clang-uml-$(PKG_VERSION).tar.gz
	docker run --cpuset-cpus=0-7 -v $(PWD):$(PWD) fedora:37 sh -c "dnf install -y make git && cd ${PWD} && make OS=fedora DIST=37 VERSION=${PKG_VERSION} COMMIT=${GIT_COMMIT} BRANCH=${GIT_BRANCH} -C packaging rpm"
