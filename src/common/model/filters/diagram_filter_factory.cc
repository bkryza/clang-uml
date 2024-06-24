/**
 * @file src/common/model/filters/diagram_filter_factory.cc
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

#include "diagram_filter_factory.h"

namespace clanguml::common::model {

namespace {
using specializations_filter_t =
    edge_traversal_filter<class_diagram::model::diagram,
        class_diagram::model::class_, common::string_or_regex>;

using class_dependants_filter_t =
    edge_traversal_filter<class_diagram::model::diagram,
        class_diagram::model::class_, common::string_or_regex>;
using class_dependencies_filter_t =
    edge_traversal_filter<class_diagram::model::diagram,
        class_diagram::model::class_, common::string_or_regex>;

using package_dependants_filter_t =
    edge_traversal_filter<package_diagram::model::diagram,
        common::model::package, common::string_or_regex>;
using package_dependencies_filter_t =
    edge_traversal_filter<package_diagram::model::diagram,
        common::model::package, common::string_or_regex>;

using source_file_dependency_filter_t =
    edge_traversal_filter<include_diagram::model::diagram,
        common::model::source_file, std::string, common::model::source_file>;
}

void basic_diagram_filter_initializer::initialize(
    const config::diagram &c, diagram_filter &df)
{
    // Process inclusive filters
    if (c.include) {
        df.add_inclusive_filter(std::make_unique<namespace_filter>(
            filter_t::kInclusive, c.include().namespaces));

        df.add_inclusive_filter(std::make_unique<modules_filter>(
            filter_t::kInclusive, c.include().modules));

        df.add_inclusive_filter(std::make_unique<module_access_filter>(
            filter_t::kInclusive, c.include().module_access));

        df.add_inclusive_filter(std::make_unique<relationship_filter>(
            filter_t::kInclusive, c.include().relationships));

        df.add_inclusive_filter(std::make_unique<access_filter>(
            filter_t::kInclusive, c.include().access));

        df.add_inclusive_filter(std::make_unique<paths_filter>(
            filter_t::kInclusive, c.root_directory(), c.include().paths));

        df.add_inclusive_filter(
            std::make_unique<class_method_filter>(filter_t::kInclusive,
                std::make_unique<access_filter>(
                    filter_t::kInclusive, c.include().access),
                std::make_unique<method_type_filter>(
                    filter_t::kInclusive, c.include().method_types)));

        df.add_inclusive_filter(
            std::make_unique<class_member_filter>(filter_t::kInclusive,
                std::make_unique<access_filter>(
                    filter_t::kInclusive, c.include().access)));

        // Include any of these matches even if one them does not match
        std::vector<std::unique_ptr<filter_visitor>> element_filters;

        element_filters.emplace_back(std::make_unique<element_filter>(
            filter_t::kInclusive, c.include().elements));

        element_filters.emplace_back(std::make_unique<element_type_filter>(
            filter_t::kInclusive, c.include().element_types));

        if (c.type() == diagram_t::kClass) {
            element_filters.emplace_back(std::make_unique<subclass_filter>(
                filter_t::kInclusive, c.include().subclasses));

            element_filters.emplace_back(std::make_unique<parents_filter>(
                filter_t::kInclusive, c.include().parents));

            element_filters.emplace_back(
                std::make_unique<specializations_filter_t>(filter_t::kInclusive,
                    relationship_t::kInstantiation,
                    c.include().specializations));

            element_filters.emplace_back(
                std::make_unique<class_dependants_filter_t>(
                    filter_t::kInclusive, relationship_t::kDependency,
                    c.include().dependants));

            element_filters.emplace_back(
                std::make_unique<class_dependencies_filter_t>(
                    filter_t::kInclusive, relationship_t::kDependency,
                    c.include().dependencies, true));
        }
        else if (c.type() == diagram_t::kSequence) {
            element_filters.emplace_back(std::make_unique<callee_filter>(
                filter_t::kInclusive, c.include().callee_types));
        }
        else if (c.type() == diagram_t::kPackage) {
            element_filters.emplace_back(
                std::make_unique<package_dependants_filter_t>(
                    filter_t::kInclusive, relationship_t::kDependency,
                    c.include().dependants));

            element_filters.emplace_back(
                std::make_unique<package_dependencies_filter_t>(
                    filter_t::kInclusive, relationship_t::kDependency,
                    c.include().dependencies, true));
        }
        else if (c.type() == diagram_t::kInclude) {
            std::vector<std::string> dependants;
            std::vector<std::string> dependencies;

            for (auto &&path : c.include().dependants) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    const std::filesystem::path dep_path{*p};
                    dependants.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            for (auto &&path : c.include().dependencies) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    const std::filesystem::path dep_path{*p};
                    dependencies.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            element_filters.emplace_back(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kInclusive, relationship_t::kAssociation,
                    dependants, false));

            element_filters.emplace_back(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kInclusive, relationship_t::kAssociation,
                    dependencies, true));
        }

        element_filters.emplace_back(std::make_unique<context_filter>(
            filter_t::kInclusive, c.include().context));

        df.add_inclusive_filter(std::make_unique<anyof_filter>(
            filter_t::kInclusive, std::move(element_filters)));
    }

    // Process exclusive filters
    if (c.exclude) {
        df.add_exclusive_filter(std::make_unique<namespace_filter>(
            filter_t::kExclusive, c.exclude().namespaces));

        df.add_exclusive_filter(std::make_unique<modules_filter>(
            filter_t::kExclusive, c.exclude().modules));

        df.add_exclusive_filter(std::make_unique<module_access_filter>(
            filter_t::kExclusive, c.exclude().module_access));

        df.add_exclusive_filter(std::make_unique<paths_filter>(
            filter_t::kExclusive, c.root_directory(), c.exclude().paths));

        df.add_exclusive_filter(std::make_unique<element_filter>(
            filter_t::kExclusive, c.exclude().elements));

        df.add_exclusive_filter(std::make_unique<element_type_filter>(
            filter_t::kExclusive, c.exclude().element_types));

        df.add_exclusive_filter(std::make_unique<relationship_filter>(
            filter_t::kExclusive, c.exclude().relationships));

        df.add_exclusive_filter(std::make_unique<access_filter>(
            filter_t::kExclusive, c.exclude().access));

        df.add_exclusive_filter(
            std::make_unique<class_method_filter>(filter_t::kExclusive,
                std::make_unique<access_filter>(
                    filter_t::kExclusive, c.exclude().access),
                std::make_unique<method_type_filter>(
                    filter_t::kExclusive, c.exclude().method_types)));

        df.add_exclusive_filter(
            std::make_unique<class_member_filter>(filter_t::kExclusive,
                std::make_unique<access_filter>(
                    filter_t::kExclusive, c.exclude().access)));

        df.add_exclusive_filter(std::make_unique<subclass_filter>(
            filter_t::kExclusive, c.exclude().subclasses));

        df.add_exclusive_filter(std::make_unique<parents_filter>(
            filter_t::kExclusive, c.exclude().parents));

        df.add_exclusive_filter(
            std::make_unique<specializations_filter_t>(filter_t::kExclusive,
                relationship_t::kInstantiation, c.exclude().specializations));

        if (c.type() == diagram_t::kClass) {
            df.add_exclusive_filter(std::make_unique<class_dependants_filter_t>(
                filter_t::kExclusive, relationship_t::kDependency,
                c.exclude().dependants));

            df.add_exclusive_filter(
                std::make_unique<class_dependencies_filter_t>(
                    filter_t::kExclusive, relationship_t::kDependency,
                    c.exclude().dependencies, true));
        }
        else if (c.type() == diagram_t::kSequence) {
            df.add_exclusive_filter(std::make_unique<callee_filter>(
                filter_t::kExclusive, c.exclude().callee_types));
        }
        else if (c.type() == diagram_t::kPackage) {
            df.add_exclusive_filter(
                std::make_unique<package_dependencies_filter_t>(
                    filter_t::kExclusive, relationship_t::kDependency,
                    c.exclude().dependencies, true));

            df.add_exclusive_filter(
                std::make_unique<package_dependants_filter_t>(
                    filter_t::kExclusive, relationship_t::kDependency,
                    c.exclude().dependants));
        }
        else if (c.type() == diagram_t::kInclude) {
            std::vector<std::string> dependants;
            std::vector<std::string> dependencies;

            for (auto &&path : c.exclude().dependants) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    std::filesystem::path dep_path{*p};
                    dependants.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            for (auto &&path : c.exclude().dependencies) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    std::filesystem::path dep_path{*p};
                    dependencies.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            df.add_exclusive_filter(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kExclusive, relationship_t::kAssociation,
                    dependants, false));

            df.add_exclusive_filter(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kExclusive, relationship_t::kAssociation,
                    dependencies, true));
        }

        df.add_exclusive_filter(std::make_unique<context_filter>(
            filter_t::kExclusive, c.exclude().context));
    }
}

void advanced_diagram_filter_initializer::initialize(
    const config::diagram &c, diagram_filter &df)
{
}

} // namespace clanguml::common::model
