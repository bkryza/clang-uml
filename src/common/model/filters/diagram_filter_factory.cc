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

void basic_diagram_filter_initializer::initialize()
{
    // Process inclusive filters
    if (diagram_config.include) {
        df.add_inclusive_filter(std::make_unique<namespace_filter>(
            filter_t::kInclusive, diagram_config.include().namespaces));

        df.add_inclusive_filter(std::make_unique<modules_filter>(
            filter_t::kInclusive, diagram_config.include().modules));

        df.add_inclusive_filter(std::make_unique<module_access_filter>(
            filter_t::kInclusive, diagram_config.include().module_access));

        df.add_inclusive_filter(std::make_unique<relationship_filter>(
            filter_t::kInclusive, diagram_config.include().relationships));

        df.add_inclusive_filter(std::make_unique<access_filter>(
            filter_t::kInclusive, diagram_config.include().access));

        df.add_inclusive_filter(std::make_unique<paths_filter>(
            filter_t::kInclusive, diagram_config.include().paths,
            diagram_config.root_directory()));

        df.add_inclusive_filter(
            std::make_unique<class_method_filter>(filter_t::kInclusive,
                std::make_unique<access_filter>(
                    filter_t::kInclusive, diagram_config.include().access),
                std::make_unique<method_type_filter>(filter_t::kInclusive,
                    diagram_config.include().method_types)));

        df.add_inclusive_filter(
            std::make_unique<class_member_filter>(filter_t::kInclusive,
                std::make_unique<access_filter>(
                    filter_t::kInclusive, diagram_config.include().access)));

        // Include any of these matches even if one them does not match
        std::vector<std::unique_ptr<filter_visitor>> element_filters;

        element_filters.emplace_back(std::make_unique<element_filter>(
            filter_t::kInclusive, diagram_config.include().elements));

        element_filters.emplace_back(std::make_unique<element_type_filter>(
            filter_t::kInclusive, diagram_config.include().element_types));

        if (diagram_config.type() == diagram_t::kClass) {
            element_filters.emplace_back(std::make_unique<subclass_filter>(
                filter_t::kInclusive, diagram_config.include().subclasses));

            element_filters.emplace_back(std::make_unique<parents_filter>(
                filter_t::kInclusive, diagram_config.include().parents));

            element_filters.emplace_back(
                std::make_unique<specializations_filter_t>(filter_t::kInclusive,
                    diagram_config.include().specializations,
                    relationship_t::kInstantiation));

            element_filters.emplace_back(
                std::make_unique<class_dependants_filter_t>(
                    filter_t::kInclusive, diagram_config.include().dependants,
                    relationship_t::kDependency));

            element_filters.emplace_back(
                std::make_unique<class_dependencies_filter_t>(
                    filter_t::kInclusive, diagram_config.include().dependencies,
                    relationship_t::kDependency, true));
        }
        else if (diagram_config.type() == diagram_t::kSequence) {
            element_filters.emplace_back(std::make_unique<callee_filter>(
                filter_t::kInclusive, diagram_config.include().callee_types));
        }
        else if (diagram_config.type() == diagram_t::kPackage) {
            element_filters.emplace_back(
                std::make_unique<package_dependants_filter_t>(
                    filter_t::kInclusive, diagram_config.include().dependants,
                    relationship_t::kDependency));

            element_filters.emplace_back(
                std::make_unique<package_dependencies_filter_t>(
                    filter_t::kInclusive, diagram_config.include().dependencies,
                    relationship_t::kDependency, true));
        }
        else if (diagram_config.type() == diagram_t::kInclude) {
            std::vector<std::string> dependants;
            std::vector<std::string> dependencies;

            for (auto &&path : diagram_config.include().dependants) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    const std::filesystem::path dep_path{*p};
                    dependants.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            for (auto &&path : diagram_config.include().dependencies) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    const std::filesystem::path dep_path{*p};
                    dependencies.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            element_filters.emplace_back(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kInclusive, dependants,
                    relationship_t::kAssociation, false));

            element_filters.emplace_back(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kInclusive, dependencies,
                    relationship_t::kAssociation, true));
        }

        element_filters.emplace_back(std::make_unique<context_filter>(
            filter_t::kInclusive, diagram_config.include().context));

        df.add_inclusive_filter(std::make_unique<anyof_filter>(
            filter_t::kInclusive, std::move(element_filters)));
    }

    // Process exclusive filters
    if (diagram_config.exclude) {
        df.add_exclusive_filter(std::make_unique<namespace_filter>(
            filter_t::kExclusive, diagram_config.exclude().namespaces));

        df.add_exclusive_filter(std::make_unique<modules_filter>(
            filter_t::kExclusive, diagram_config.exclude().modules));

        df.add_exclusive_filter(std::make_unique<module_access_filter>(
            filter_t::kExclusive, diagram_config.exclude().module_access));

        df.add_exclusive_filter(std::make_unique<paths_filter>(
            filter_t::kExclusive, diagram_config.exclude().paths,
            diagram_config.root_directory()));

        df.add_exclusive_filter(std::make_unique<element_filter>(
            filter_t::kExclusive, diagram_config.exclude().elements));

        df.add_exclusive_filter(std::make_unique<element_type_filter>(
            filter_t::kExclusive, diagram_config.exclude().element_types));

        df.add_exclusive_filter(std::make_unique<relationship_filter>(
            filter_t::kExclusive, diagram_config.exclude().relationships));

        df.add_exclusive_filter(std::make_unique<access_filter>(
            filter_t::kExclusive, diagram_config.exclude().access));

        df.add_exclusive_filter(
            std::make_unique<class_method_filter>(filter_t::kExclusive,
                std::make_unique<access_filter>(
                    filter_t::kExclusive, diagram_config.exclude().access),
                std::make_unique<method_type_filter>(filter_t::kExclusive,
                    diagram_config.exclude().method_types)));

        df.add_exclusive_filter(
            std::make_unique<class_member_filter>(filter_t::kExclusive,
                std::make_unique<access_filter>(
                    filter_t::kExclusive, diagram_config.exclude().access)));

        df.add_exclusive_filter(std::make_unique<subclass_filter>(
            filter_t::kExclusive, diagram_config.exclude().subclasses));

        df.add_exclusive_filter(std::make_unique<parents_filter>(
            filter_t::kExclusive, diagram_config.exclude().parents));

        df.add_exclusive_filter(std::make_unique<specializations_filter_t>(
            filter_t::kExclusive, diagram_config.exclude().specializations,
            relationship_t::kInstantiation));

        if (diagram_config.type() == diagram_t::kClass) {
            df.add_exclusive_filter(std::make_unique<class_dependants_filter_t>(
                filter_t::kExclusive, diagram_config.exclude().dependants,
                relationship_t::kDependency));

            df.add_exclusive_filter(
                std::make_unique<class_dependencies_filter_t>(
                    filter_t::kExclusive, diagram_config.exclude().dependencies,
                    relationship_t::kDependency, true));
        }
        else if (diagram_config.type() == diagram_t::kSequence) {
            df.add_exclusive_filter(std::make_unique<callee_filter>(
                filter_t::kExclusive, diagram_config.exclude().callee_types));
        }
        else if (diagram_config.type() == diagram_t::kPackage) {
            df.add_exclusive_filter(
                std::make_unique<package_dependencies_filter_t>(
                    filter_t::kExclusive, diagram_config.exclude().dependencies,
                    relationship_t::kDependency, true));

            df.add_exclusive_filter(
                std::make_unique<package_dependants_filter_t>(
                    filter_t::kExclusive, diagram_config.exclude().dependants,
                    relationship_t::kDependency));
        }
        else if (diagram_config.type() == diagram_t::kInclude) {
            std::vector<std::string> dependants;
            std::vector<std::string> dependencies;

            for (auto &&path : diagram_config.exclude().dependants) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    std::filesystem::path dep_path{*p};
                    dependants.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            for (auto &&path : diagram_config.exclude().dependencies) {
                if (auto p = path.get<std::string>(); p.has_value()) {
                    std::filesystem::path dep_path{*p};
                    dependencies.emplace_back(
                        dep_path.lexically_normal().string());
                }
            }

            df.add_exclusive_filter(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kExclusive, dependants,
                    relationship_t::kAssociation, false));

            df.add_exclusive_filter(
                std::make_unique<source_file_dependency_filter_t>(
                    filter_t::kExclusive, dependencies,
                    relationship_t::kAssociation, true));
        }

        df.add_exclusive_filter(std::make_unique<context_filter>(
            filter_t::kExclusive, diagram_config.exclude().context));
    }
}

template <>
void advanced_diagram_filter_initializer::add_filter<
    source_file_dependency_filter_t>(const filter_t &filter_type,
    const std::vector<common::string_or_regex> &filter_config,
    std::vector<std::unique_ptr<filter_visitor>> &result, relationship_t &&rt,
    bool &&direction)
{
    std::vector<std::string> deps;
    for (auto &&path : filter_config) {
        if (auto p = path.get<std::string>(); p.has_value()) {
            const std::filesystem::path dep_path{*p};
            deps.emplace_back(dep_path.lexically_normal().string());
        }
    }

    result.emplace_back(std::make_unique<source_file_dependency_filter_t>(
        filter_type, deps, rt, direction));
}

std::vector<std::unique_ptr<filter_visitor>>
advanced_diagram_filter_initializer::build(
    filter_t filter_type, const config::filter &filter_config)
{
    std::vector<std::unique_ptr<filter_visitor>> result;

    // At any level, only allof, anyof, or a set of other non-operator
    // filters can be present
    if (filter_config.allof) {
        std::vector<std::unique_ptr<filter_visitor>> allof_filters =
            build(filter_type, *filter_config.allof);

        auto allof_filter = std::make_unique<common::model::allof_filter>(
            filter_type, std::move(allof_filters));

        result.emplace_back(std::move(allof_filter));
    }

    if (filter_config.anyof) {
        std::vector<std::unique_ptr<filter_visitor>> anyof_filters =
            build(filter_type, *filter_config.anyof);

        auto anyof_filter = std::make_unique<common::model::anyof_filter>(
            filter_type, std::move(anyof_filters));

        result.emplace_back(std::move(anyof_filter));
    }

    add_filter<namespace_filter>(filter_type, filter_config.namespaces, result);
    add_filter<modules_filter>(filter_type, filter_config.modules, result);
    add_filter<module_access_filter>(
        filter_type, filter_config.module_access, result);
    add_filter<relationship_filter>(
        filter_type, filter_config.relationships, result);
    add_filter<access_filter>(filter_type, filter_config.access, result);
    add_filter<method_type_filter>(
        filter_type, filter_config.method_types, result);

    add_filter<paths_filter>(filter_type, filter_config.paths, result,
        diagram_config.root_directory());

    add_filter<element_filter>(filter_type, filter_config.elements, result);
    add_filter<element_type_filter>(
        filter_type, filter_config.element_types, result);
    add_filter<subclass_filter>(filter_type, filter_config.subclasses, result);
    add_filter<parents_filter>(filter_type, filter_config.parents, result);

    add_filter<specializations_filter_t>(filter_type,
        filter_config.specializations, result, relationship_t::kInstantiation,
        false);
    add_filter<class_dependants_filter_t>(filter_type, filter_config.dependants,
        result, relationship_t::kDependency, false);
    add_filter<class_dependencies_filter_t>(filter_type,
        filter_config.dependencies, result, relationship_t::kDependency, true);

    add_filter<callee_filter>(filter_type, filter_config.callee_types, result);

    add_filter<package_dependants_filter_t>(filter_type,
        filter_config.dependants, result, relationship_t::kDependency, false);
    add_filter<package_dependencies_filter_t>(filter_type,
        filter_config.dependencies, result, relationship_t::kDependency, true);

    add_filter<context_filter>(filter_type, filter_config.context, result);

    if (diagram_config.type() == diagram_t::kInclude) {
        add_filter<source_file_dependency_filter_t>(filter_type,
            filter_config.dependants, result, relationship_t::kAssociation,
            false);
        add_filter<source_file_dependency_filter_t>(filter_type,
            filter_config.dependencies, result, relationship_t::kAssociation,
            true);
    }

    return result;
}

void advanced_diagram_filter_initializer::initialize()
{
    if (diagram_config.include) {
        auto inclusive_filter =
            build(filter_t::kInclusive, diagram_config.include());
        for (auto &f : inclusive_filter)
            df.add_filter(filter_t::kInclusive, std::move(f));
    }

    if (diagram_config.exclude) {
        auto exclusive_filter =
            build(filter_t::kExclusive, diagram_config.exclude());
        for (auto &f : exclusive_filter)
            df.add_filter(filter_t::kExclusive, std::move(f));
    }
}

} // namespace clanguml::common::model
