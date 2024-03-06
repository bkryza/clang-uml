# Quick start

<!-- toc -->



<!-- tocstop -->

To add an initial class diagram to your project, follow these steps:

1. Enter your project's top level directory and run:
    ```bash
    clang-uml --init
    ```
2. Edit the generated `.clang-uml` file and set the following:
    ```yaml
    # Path to `compile_commands.json` directory
    compilation_database_dir: .
    # Path to diagram output directory
    output_directory: diagrams
    diagrams:
      # This is the name of the diagram
      some_class_diagram:
        type: class
        # Parse only translation units in `src` subdirectory
        glob:
          - src/*.cc
        # Render all names relative to `myproject` namespace
        using_namespace: myproject
        include:
          # Include only elements in `myproject` namespace
          namespaces:
            - myproject
        exclude:
          # Exclude elements in `myproject::detail` namespace
          namespaces:
            - myproject::detail
      ```
3. Run `clang-uml` in the project's top directory:
    ```bash
    clang-uml
    # or to see generation progress for each diagram
    clang-uml --progress
    ```
4. Generate SVG images from the PlantUML diagrams:
    ```bash
   plantuml -tsvg diagrams/*.puml
   ```
   or generate also MermaidJS diagram (requires [mermaid-cli](https://github.com/mermaid-js/mermaid-cli)):
   ```
   clang-uml --progress -n some_class_diagram -g mermaid
   mmdc -i diagrams/some_class_diagram.mmd -o diagrams/some_class_diagram.svg
   ```
   
   Steps 3 and 4 can be combined into one step:
   ```
   clang-uml -p -n some_class_diagram -g plantuml -r --plantuml-cmd="/usr/bin/plantuml -tsvg diagrams/{}.puml"
   ```
   where `-r` enables diagram rendering and `--plantuml-cmd` specifies command
   to execute on each generated diagram.
5. Add another diagram:
   ```bash
   clang-uml --add-sequence-diagram another_diagram
   ```
6. Now list the diagrams defined in the config file:
   ```bash
   clang-uml -l
   The following diagrams are defined in the config file:
     - another_diagram [sequence]
     - some_class_diagram [class]
   ```
7. Generate only the new diagram in JSON format:
   ```bash
   clang-uml -n another_diagram -g json
   ```
