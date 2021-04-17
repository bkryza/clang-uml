#!/usr/bin/python3

##
## util/generate_test_cases_docs.py
##
## Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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


import yaml
from shutil import copyfile

with open(r'tests/test_cases.yaml') as f:
    test_groups = yaml.full_load(f)['test_cases']

    # Generate test_cases.md index
    with open(r'docs/test_cases.md', 'w') as tc_index:
        tc_index.write("# Test cases index\n")
        for test_group, test_cases in test_groups.items():
            tc_index.write("## {}\n".format(test_group))
            for test_case in test_cases:
                tc_index.write(" * [{0}](./test_cases/{0}.md) - {1}\n".format(
                    test_case['name'], test_case['title']))

    # Generate invididual documentation docs
    for test_group, test_cases in test_groups.items():
        for test_case in test_cases:
            name = test_case['name']
            with open(r'docs/test_cases/{}.md'.format(name), 'w') as tc:
                tc.write("# {} - {}\n".format(name, test_case['title']))

                # Write out test description
                if test_case['description']:
                    tc.write("{}\n".format(test_case['description']))

                # Write test config file
                config = open('tests/{0}/.clanguml'.format(name), 'r').read()
                tc.write("## Config\n")
                tc.write("```yaml\n")
                tc.write(config)
                tc.write("\n```\n")
                tc.write("## Source code\n")
                tc.write("```cpp\n")
                tc.write(open('tests/{0}/{0}.cc'.format(name), 'r').read())
                tc.write("\n```\n")

                # Copy and link the diagram image
                config_dict = yaml.full_load(config)
                tc.write("## Generated UML diagrams\n")
                for diagram_name, _ in config_dict['diagrams'].items():
                    copyfile('debug/tests/puml/{}.png'.format(diagram_name),
                             'docs/test_cases/{}.png'.format(diagram_name))
                    tc.write("![{0}](./{0}.png \"{1}\")\n".format(
                        diagram_name, test_case["title"]))
