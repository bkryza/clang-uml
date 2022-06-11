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

SHELL := /bin/bash
.ONESHELL:

.PHONY: download deb clean conda

NAME ?= clang-uml
REBUILD ?= 1
MAINTAINER_NAME ?= Bartek Kryza
MAINTAINER_EMAIL ?= bkryza@gmail.com
GPG_KEY ?= 702014E322FE5CA9B5D920F66CDA4566635E93B1
OS ?= ubuntu
DIST ?= focal
TAR_EXT ?= gz

build_dir = _BUILD/$(NAME)/$(OS)/$(DIST)

VERSION	?= $(shell git describe --tags --always --abbrev=7)
COMMIT ?= $(shell git rev-parse HEAD)
BRANCH ?= $(shell git rev-parse --abbrev-ref HEAD)

#
# Replace mustache template variable in all files in directory recursively,
# e.g.:
#  $(call subst_template,VERSION,${VERSION},debian)
#
define subst_template_dir
        find $(3) -type f -exec sed -i "s/{{$(1)}}/$(2)/g" {} \;
endef

define subst_conda_meta_yaml
        find $(3) -name meta.yaml -exec sed -i "s/{{$(1)}}/$(2)/g" {} \;
endef

deb:
		rm -rf $(build_dir)
		mkdir -p $(build_dir)
		git-archive-all --prefix=clang-uml-$(VERSION)/ $(build_dir)/clang-uml-$(VERSION).tar.$(TAR_EXT)
		cd $(build_dir)
		echo "Creating archive from current source"
		echo "Creating directory: ", $(NAME)-$(VERSION)
		mkdir $(NAME)-$(VERSION)
		echo "Extracting source archive..."
		tar xf $(NAME)-$(VERSION).tar.$(TAR_EXT) -C $(NAME)-$(VERSION) --strip-components 1
		cp -R ../../../../debian $(NAME)-$(VERSION)/debian
		cd $(NAME)-$(VERSION)
		$(call subst_template_dir,DATETIME,$(shell date -R),debian)
		$(call subst_template_dir,OS,${OS},debian)
		$(call subst_template_dir,NAME,${NAME},debian)
		$(call subst_template_dir,VERSION,${VERSION},debian)
		$(call subst_template_dir,REBUILD,${REBUILD},debian)
		$(call subst_template_dir,DISTRIBUTION,${DIST},debian)
		$(call subst_template_dir,MAINTAINER_NAME,${MAINTAINER_NAME},debian)
		$(call subst_template_dir,MAINTAINER_EMAIL,${MAINTAINER_EMAIL},debian)
		mk-origtargz ../$(NAME)-$(VERSION).tar.$(TAR_EXT)
		# BUILD SOURCE PACKAGE FOR LAUNCHPAD
		debuild -S -sa -us -d -k$(GPG_KEY) #--source-option=--include-binaries --source-option=--include-removal --source-option=-isession
		# BUILD LOCALLY BINARY PACKAGE
		#debuild -us -uc #--source-option=--include-binaries --source-option=--include-removal --source-option=-isession

conda:
		mkdir -p _BUILD/conda
		git-archive-all --prefix=clang-uml-$(VERSION)/ _BUILD/conda/clang-uml-$(VERSION).tar.$(TAR_EXT)
		cp conda/meta.yaml.in conda/meta.yaml
		$(call subst_conda_meta_yaml,PKG_VERSION,${VERSION},conda)
		$(call subst_conda_meta_yaml,PKG_SOURCE,..\/_BUILD\/conda\/clang-uml-$(VERSION).tar.$(TAR_EXT),conda)
		$(call subst_conda_meta_yaml,GIT_COMMIT,${COMMIT},conda)
		$(call subst_conda_meta_yaml,GIT_BRANCH,${BRANCH},conda)
		conda build conda
