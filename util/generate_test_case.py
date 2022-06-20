#!/usr/bin/python3

##
## util/generate_test_case.py
##
## Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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
import sys
import jinja2

TEST_CASE_MULTIPLIER = 10000

CLASS_DIAGRAM_TEST_CASE_EXAMPLES = """
    // Check if all classes exist
    //REQUIRE_THAT(puml, IsClass(_A("A")));
    
    // Check if class templates exist
    //REQUIRE_THAT(puml, IsClassTemplate("A", "T,P,CMP,int N"));
    
    // Check if all enums exist
    //REQUIRE_THAT(puml, IsEnum(_A("Lights")));
    
    // Check if all inner classes exist
    //REQUIRE_THAT(puml, IsInnerClass(_A("A"), _A("AA")));

    // Check if all inheritance relationships exist
    //REQUIRE_THAT(puml, IsBaseClass(_A("Base"), _A("Child")));
    
    // Check if all methods exist
    //REQUIRE_THAT(puml, (IsMethod<Public, Const>("foo")));
    
    // Check if all fields exist
    //REQUIRE_THAT(puml, (IsField<Private>("private_member", "int")));
    
    // Check if all relationships exist
    //REQUIRE_THAT(puml, IsAssociation(_A("D"), _A("A"), "-as"));
    //REQUIRE_THAT(puml, IsDependency(_A("R"), _A("B")));
    //REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("D")));
    //REQUIRE_THAT(puml, IsComposition(_A("R"), _A("D")));
    //REQUIRE_THAT(puml, IsInstantiation(_A("ABCD::F<T>"), _A("F<int>")));
"""

SEQUENCE_DIAGRAM_TEST_CASE_EXAMPLES = """
    // Check if all calls exist
    //REQUIRE_THAT(puml, HasCall("A", "log_result"));
    //REQUIRE_THAT(puml, HasCallWithResponse("B", "A", "add3"));
"""

PACKAGE_DIAGRAM_TEST_CASE_EXAMPLES = """
    // Check if all packages exist
    //REQUIRE_THAT(puml, IsPackage("ns1"));
"""

INCLUDE_DIAGRAM_TEST_CASE_EXAMPLES = """
    // Check all folders exist
    //REQUIRE_THAT(puml, IsFolder("lib1"));
    
    // Check if all files exist
    //REQUIRE_THAT(puml, IsFile("lib1.h"));
    
    // Check if all includes exists
    //REQUIRE_THAT(puml, IsAssociation(_A("t40002.cc"), _A("lib1.h")));
    //REQUIRE_THAT(puml, IsDependency(_A("t40001_include1.h"), _A("string")));
"""

def test_case_already_exists(name):
    return os.path.isdir(os.path.join(os.path.dirname(__file__), '..', name))


def test_case_category(type):
    if type == 'class':
        return 0
    elif type == 'sequence':
        return 2 * TEST_CASE_MULTIPLIER
    elif type == 'package':
        return 3 * TEST_CASE_MULTIPLIER
    elif type == 'include':
        return 4 * TEST_CASE_MULTIPLIER
    else:
        print(f"Invalid test case type '{type}' - aborting...")
        sys.exit(1)


def examples_for_type(type):
    if type == 'class':
        return CLASS_DIAGRAM_TEST_CASE_EXAMPLES
    elif type == 'sequence':
        return SEQUENCE_DIAGRAM_TEST_CASE_EXAMPLES
    elif type == 'package':
        return PACKAGE_DIAGRAM_TEST_CASE_EXAMPLES
    elif type == 'include':
        return INCLUDE_DIAGRAM_TEST_CASE_EXAMPLES
    else:
        print(f"Invalid test case type '{type}' - aborting...")
        sys.exit(1)


def generate(environment, variables, src, dst, dst_path):
    template = environment.get_template(src)

    output = template.render(variables)

    with open(os.path.join(dst_path, dst), 'wb+') as fh:
        fh.write(output.encode("UTF-8"))


def main(args):
    if len(args) != 2:
        print("Usage: ./generate_test_cases.py <type> <number>")

    test_case_type = args[0]
    test_case_number = int(args[1])
    test_case_name = f't{test_case_category(test_case_type) + test_case_number:05d}'

    if test_case_already_exists(test_case_name):
        print(f"Test case 'test_case_name' already exists - aborting...")
        sys.exit(1)

    template_directory = os.path.join(os.path.dirname(__file__), 'templates', 'test_cases')

    loader = jinja2.FileSystemLoader(searchpath=template_directory)

    env_parameters = dict(
        loader=loader,
        undefined=jinja2.StrictUndefined
    )
    environment = jinja2.Environment(**env_parameters)

    test_case_directory = os.path.join(os.path.dirname(__file__), '..', 'tests', test_case_name)

    os.mkdir(test_case_directory)

    examples = examples_for_type(test_case_type)

    variables = dict(type = test_case_type, name = test_case_name, examples = examples)

    generate(environment, variables, '.clang-uml', '.clang-uml', test_case_directory)
    generate(environment, variables, 't00000.cc', f'{test_case_name}.cc', test_case_directory)
    generate(environment, variables, 'test_case.h', 'test_case.h', test_case_directory)


if __name__ == '__main__':
    main(sys.argv[1:])
