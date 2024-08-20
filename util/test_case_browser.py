#!/usr/bin/env python3

##
## util/test_case_browser.py
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

from nicegui import ui
import os
import sys


def get_svg_files(directory):
    """Return a list of SVG files in the given directory."""
    return sorted([f for f in os.listdir(directory) if f.endswith('.svg')])


def main(dir1, dir2):
    # Get SVG files from both directories
    files1 = get_svg_files(dir1)
    files2 = get_svg_files(dir2)

    # Find common files
    common_files = sorted(list(set(files1) & set(files2)))

    if not common_files:
        ui.label('No common SVG files found in the provided directories.')
        return

    current_index = {'index': 0}

    def update_images():
        """Update the displayed SVG images based on the current index."""
        if current_index['index'] < 0:
            current_index['index'] = 0
        if current_index['index'] >= len(common_files):
            current_index['index'] = len(common_files) - 1

        left_image.source = os.path.join(dir1, common_files[current_index['index']])
        right_image.source = os.path.join(dir2, common_files[current_index['index']])
        title_label.text = f'Comparing: {common_files[current_index["index"]]}'
        dropdown.value = common_files[current_index['index']]

    def next_image():
        """Go to the next image."""
        if current_index['index'] < len(common_files) - 1:
            current_index['index'] += 1
        else:
            current_index['index'] = 0
        update_images()

    def prev_image():
        """Go to the previous image."""
        if current_index['index'] > 0:
            current_index['index'] -= 1
        else:
            current_index['index'] = len(common_files) - 1

        update_images()

    def select_image(selected_file):
        """Switch to the selected image."""
        current_index['index'] = common_files.index(selected_file)
        update_images()

    def handle_key(event):
        """Handle key press events."""
        if event.action.keydown:
            if event.key.arrow_right:
                next_image()
            elif event.key.arrow_left:
                prev_image()

    with ui.header().style('height: 5vh; display: flex; align-items: center; padding: 0 1rem;'):
        ui.image('https://raw.githubusercontent.com/bkryza/clang-uml/master/docs/img/clang-uml-logo.svg').style('width: 300px')
        ui.label('Test Case Diagram Viewer').classes('text-4xl ml-4')

    with ui.row().classes('justify-center'):
        ui.button('Previous', on_click=prev_image)
        ui.button('Next', on_click=next_image)

    dropdown = ui.select(common_files, on_change=lambda e: select_image(e.value)).classes('mb-4')

    title_label = ui.label().classes('text-xl font-semibold text-center mb-4')

    with ui.grid(columns=2).classes('w-full gap-2'):
        left_image = ui.image().classes('border p-1')
        right_image = ui.image().classes('border p-1')

    keyboard = ui.keyboard(on_key=handle_key)

    update_images()


if __name__ in {"__main__", "__mp_main__"}:
    if len(sys.argv) != 3:
        print("Usage: test_case_browser.py <directory1> <directory2>")
    else:
        main(sys.argv[1], sys.argv[2])
        ui.run()
