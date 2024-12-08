#!/usr/bin/env python3

##
## util/validate_graphml.py
##
## Copyright (c) 2024 Bartek Kryza <bkryza@gmail.com>
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
import xmlschema


def validate_xml(xml_path, schema):
    if not schema.is_valid(xml_path):
        errors = schema.validate(xml_path, lazy=True)
        for error in errors:
            print(f"GraphML schema error: {error}")

        raise Exception("Invalid schema")



if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <path_to_xml_file>")
        sys.exit(1)

    xml_file_path = sys.argv[1]
    schema = xmlschema.XMLSchema(
        "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd")

    files = sys.argv[1:]

    ok = 0
    for f in files:
        try:
            validate_xml(f, schema)
            print(f'File {f} is valid')
        except xmlschema.XMLSchemaException as e:
            print(f"File {f} is invalid - schema error: {e}")
            ok = 1
        except Exception as e:
            print(f"File {f} is invalid - an error occurred: {e}")
            ok = 1

    sys.exit(ok)