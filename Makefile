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

.PHONY: clean
clean:
	rm -rf debug release

debug/CMakeLists.txt:
	cmake -S . -B debug \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Debug

release/CMakeLists.txt:
	cmake -S . -B release \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_BUILD_TYPE=Release

debug: debug/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	make -C debug -j$(NUMPROC)

release: release/CMakeLists.txt
	make -C release -j

test: debug
	CTEST_OUTPUT_ON_FAILURE=1 make -C debug test

test_plantuml: test
	plantuml debug/tests/puml/*.puml

document_test_cases: test_plantuml
	python3 util/generate_test_cases_docs.py

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
