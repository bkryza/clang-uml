# docker/Makefile
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

.PHONY: builder_docker
builder_docker:
	docker build -f Dockerfile.builder -t bkryza/clang-uml-builder:v1 .

.PHONY: builder_cacche_volume
builder_cacche_volume:
	docker volume create clanguml_ccache