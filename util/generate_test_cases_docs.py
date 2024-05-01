#!/usr/bin/python3

##
## util/generate_test_cases_docs.py
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

import os
import yaml
from shutil import copyfile


with open(r'tests/test_cases.yaml') as f:
    test_groups = yaml.full_load(f)['test_cases']

    # Generate test_cases.md index
    with open(r'docs/test_cases.md', 'w') as tc_index:
        tc_index.write('# Test cases index\n')
        tc_index.write("* [Class diagrams](#class-diagrams)\n")
        tc_index.write("* [Sequence diagrams](#sequence-diagrams)\n")
        tc_index.write("* [Package diagrams](#package-diagrams)\n")
        tc_index.write("* [Include diagrams](#include-diagrams)\n")
        tc_index.write("* [Configuration diagrams](#configuration-diagrams)\n")
        tc_index.write("\n")
        for test_group, test_cases in test_groups.items():
            tc_index.write(f'## {test_group}\n')
            for test_case in test_cases:
                tc_index.write(f' * [{test_case["name"]}](./test_cases/{test_case["name"]}.md) - {test_case["title"]}\n')

    # Generate invididual documentation docs
    for test_group, test_cases in test_groups.items():
        for test_case in test_cases:
            name = test_case['name']
            with open(f'docs/test_cases/{name}.md', 'w') as tc:
                tc.write(f'# {name} - {test_case["title"]}\n')

                # Write out test description
                if test_case['description']:
                    tc.write(f'{test_case["description"]}\n')

                # Write test config file
                config = open(f'tests/{name}/.clang-uml', 'r').read()
                tc.write("## Config\n")
                tc.write("```yaml\n")
                tc.write(config)
                tc.write("\n```\n")
                tc.write("## Source code\n")
                for root, dirs, files in os.walk(f'tests/{name}/'):
                    for source_file in files:
                        if source_file.endswith((
                                ".h", ".cc", ".c", ".cppm", ".cu", ".cuh")):
                            if source_file == "test_case.h":
                                continue
                            file_path = os.path.join(root, source_file)
                            tc.write(f'File `{file_path}`\n')
                            tc.write("```cpp\n")
                            with open(file_path, 'r') as file:
                                tc.write(file.read())
                            tc.write("\n```\n")

                # Copy and link the diagram image
                config_dict = yaml.full_load(config)
                tc.write("## Generated PlantUML diagrams\n")
                for diagram_name, _ in config_dict['diagrams'].items():
                    copyfile(f'debug/tests/diagrams/plantuml/{diagram_name}.svg',
                             f'docs/test_cases/{diagram_name}.svg')
                    tc.write(f'![{diagram_name}](./{diagram_name}.svg "{test_case["title"]}")\n')

                tc.write("## Generated Mermaid diagrams\n")
                for diagram_name, _ in config_dict['diagrams'].items():
                    copyfile(f'debug/tests/diagrams/mermaid/{diagram_name}.svg',
                             f'docs/test_cases/{diagram_name}_mermaid.svg')
                    tc.write(f'![{diagram_name}](./{diagram_name}_mermaid.svg "{test_case["title"]}")\n')

                tc.write("## Generated JSON models\n")
                for diagram_name, _ in config_dict['diagrams'].items():
                    jd = f'debug/tests/diagrams/{diagram_name}.json'
                    if os.path.exists(jd):
                        with open(jd) as f:
                            contents = f.read()
                            tc.write("```json\n")
                            tc.write(contents)
                            tc.write("\n```\n")