#!/usr/bin/python3

##
## util/format_svg.py
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
from lxml import etree
import lxml.html

def main(argv):
    if len(argv) < 1:
        print('Usage: \n')
        print('\t\t ./format_svg.py input_file1.svg input_file2.svg ...\n')
        sys.exit(1)

    for inputfile in argv:
        # Read svg file contents
        with open(inputfile, 'r') as f:
            xml = f.read()

        # Parse SVG XML
        tree = etree.fromstring(bytes(xml, encoding='utf8'))

        if not tree.attrib['width'] or tree.attrib['width'].endswith('%'):
            # Make sure the width is equal to the viewBox width
            tree.attrib['width'] = tree.attrib['viewBox'].split(' ')[2]

        # Add style color for <a> links
        defs = tree.xpath('//svg:defs', namespaces={'svg':'http://www.w3.org/2000/svg'})
        if defs:
            style = etree.SubElement(defs[0], 'style')
            style.text = 'a:hover { text-decoration: underline; }'
            style.set('type', 'text/css')

        # Remove comments from SVG, to minimize diff
        # when updating diagrams in Git
        comments = tree.xpath('//comment()')

        for c in comments:
            p = c.getparent()
            p.remove(c)

        # Overwrite the input svg properly formatted
        etree.ElementTree(tree).write(inputfile, encoding='utf-8', pretty_print=True)


if __name__ == "__main__":
    main(sys.argv[1:])