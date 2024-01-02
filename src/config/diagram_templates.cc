/**
 * @file src/config/diagram_templates.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diagram_templates.h"

namespace clanguml {
namespace config {

const std::string &get_predefined_diagram_templates()
{
    static const std::string predefined_diagram_templates =
        R"(# Predefined diagram templates
parents_hierarchy_tmpl:
  description: Generate class parents inheritance diagram
  type: class
  template: |
    {{ diagram_name }}:
      type: class
      {% if exists("glob") %}
      glob: [{{ glob }}]
      {% endif %}
      {% if exists("using_namespace") %}
      using_namespace: {{ using_namespace }}
      {% endif %}
      include:
        parents: [{{ class_name }}]
        namespaces: [{{ namespace_name }}]
      relationships:
        - inheritance
      exclude:
        access: [public, protected, private]
      plantuml:
        before:
          - left to right direction
subclass_hierarchy_tmpl:
  description: Generate class children inheritance diagram
  type: class
  template: |
    {{ diagram_name }}:
      type: class
      {% if exists("glob") %}
      glob: [{{ glob }}]
      {% endif %}
      {% if exists("using_namespace") %}
      using_namespace: {{ using_namespace }}
      {% endif %}
      include:
        parents: [{{ class_name }}]
        namespaces: [{{ namespace_name }}]
      relationships:
        - inheritance
      exclude:
        access: [public, protected, private]
      plantuml:
        before:
          - left to right direction
class_context_tmpl:
  description: Generate class context diagram
  type: class
  template: |
    "{{ diagram_name }}":
      type: class
      {% if exists("glob") %}
      glob: [{{ glob }}]
      {% endif %}
      {% if exists("using_namespace") %}
      using_namespace: {{ using_namespace }}
      {% endif %}
      include:
        context: [{{ class_name }}]
        namespaces: [{{ namespace_name }}]
)";

    return predefined_diagram_templates;
}

} // namespace config
} // namespace clanguml
