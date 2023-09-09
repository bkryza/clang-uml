#!/usr/bin/python3

##
## util/validate_json.py
##
## Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
import subprocess

from pathlib import Path

def print_usage():
    print(f'Usage: ./generate_mermaid.py file1.mmd file2.mmd ...')


files = sys.argv[1:]


if not files:
    print_usage()
    sys.exit(1)

ok = 0

for f in files:
    try:
        print(f'Generating Mermaid diagram from {f}')
        f_svg = Path(f).with_suffix('.svg').name
        target = Path(f).parent.absolute()
        target = target.joinpath('mermaid')
        target = target.joinpath(f_svg)
        subprocess.check_call(['mmdc',  '-i', f, '-o', target])
    except subprocess.CalledProcessError:
        ok = 1
        print(f'ERROR: Generating Mermaid diagram from {f} failed')

sys.exit(ok)
