#!/bin/bash

##
## util/check_formatting.sh
##
## Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##

shopt -s globstar

include_file_list() {
  cat .clang-format-include | grep "^\+" | awk '{print $2}' \
      | tr [:space:] '\n' | sort | uniq
}

ignore_file_list() {
  echo $(cat .clang-format-include | grep "^\-" | awk '{print $2}') \
       $(git ls-files --others --exclude-standard --ignored) \
        | tr [:space:] '\n' | sort | uniq
}

valid_ignore_file_list() {
  echo $(include_file_list) $(ignore_file_list) \
       | tr [:space:] '\n' | sort | uniq -d
}

effective_file_list() {
  echo $(include_file_list) $(valid_ignore_file_list) \
       | tr [:space:] '\n' | sort | uniq -u
}

GIT_STATUS=$(git diff-index --quiet HEAD --)

EFFECTIVE_FILE_LIST=$(effective_file_list)

if [[ ${#EFFECTIVE_FILE_LIST[@]} -eq 0 ]]; then
    echo ".clang-format-include patterns did not match any files."
    exit 0
else
    clang-format-15 --dry-run --Werror ${EFFECTIVE_FILE_LIST}
fi