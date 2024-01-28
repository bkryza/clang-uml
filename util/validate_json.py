#!/usr/bin/python3

##
## util/validate_json.py
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
import sys
import json


def print_usage():
    print(f'Usage: ./validate_json.py file1.json file2.json ...')


files = sys.argv[1:]


if not files:
    print_usage()
    sys.exit(1)

ok = 0

for f in files:
    with open(f, 'r') as file:
        try:
            json.load(file)
            print(f'File {f} is valid')
        except ValueError:
            ok = 1
            print(f'File {f} is invalid')

sys.exit(ok)