# Makefile
#
# Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

NUMPROC ?= $(shell nproc)

LLVM_CONFIG_PATH ?=
CMAKE_CXX_FLAGS ?=

.PHONY: clean
clean:
	rm -rf debug release

debug/CMakeLists.txt:
	cmake -S . -B debug \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DLLVM_CONFIG_PATH=$(LLVM_CONFIG_PATH)

release/CMakeLists.txt:
	cmake -S . -B release \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DLLVM_CONFIG_PATH=$(LLVM_CONFIG_PATH)

debug: debug/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	make -C debug -j$(NUMPROC)

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
	debug/clang-uml
	plantuml -tsvg docs/diagrams/*.puml
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
	docker run --rm -v $(CURDIR):/root/sources bkryza/clang-format-check:1.3

.PHONY: format
format:
	docker run --rm -v $(CURDIR):/root/sources bkryza/clang-format-check:1.3

.PHONY: iwyu_fixes
iwyu_fixes: debug
	python3 $(shell which iwyu_tool.py) -p debug > debug/iwyu.out
	python3 $(shell which fix_includes.py) -h --re_only="${PWD}/src/.*" < debug/iwyu.out
	python3 $(shell which fix_includes.py) -h --re_only="${PWD}/tests/.*" < debug/iwyu.out
