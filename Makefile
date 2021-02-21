# Makefile
#
# Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

# Specify LLVM version
LLVM_VERSION ?= 11

.PHONY: clean
clean:
	rm -rf debug release

debug/CMakeLists.txt:
	cmake -S . -B debug \
		-DCMAKE_BUILD_TYPE=Debug \
		-DLIBCLANG_LLVM_CONFIG_EXECUTABLE=/usr/bin/llvm-config-${LLVM_VERSION}

release/CMakeLists.txt:
	cmake -S . -B release \
		-DCMAKE_BUILD_TYPE=Release \
		-DLIBCLANG_LLVM_CONFIG_EXECUTABLE=/usr/bin/llvm-config-${LLVM_VERSION}

debug: debug/CMakeLists.txt
	make -C debug -j
	make -C debug test

release: release/CMakeLists.txt
	make -C release -j

.PHONY: clang-format
clang-format:
	docker run --rm -v $(CURDIR):/root/sources bkryza/clang-format-check:1.2
