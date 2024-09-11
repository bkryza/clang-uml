/**
 * @file src/common/generators/text_based_generator.h
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
#pragma once

#include "class_diagram/model/diagram.h"
#include "common/generators/generator.h"
#include "common/generators/nested_element_stack.h"
#include "common/model/package.h"
#include "config/config.h"
#include "util/error.h"
#include "util/util.h"

#include <optional>
#include <ostream>
#include <regex>
#include <string>

namespace clanguml::class_diagram::generators {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;

using clanguml::common::model::package;

/**
 * @brief Common methods for PlantUML and MermaidJS class diagrams
 */
template <typename G> class text_diagram_strategy {
public:
    using method_groups_t =
        std::map<std::string, std::vector<model::class_method>>;

    text_diagram_strategy(G &generator, bool generate_packages)
        : generator_{generator}
        , together_group_stack_{generate_packages}
    {
    }

    void generate(const package &p, std::ostream &ostr) const
    {
        generator_.start_package(p, ostr);

        for (const auto &subpackage : p) {
            if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
                // TODO: add option - generate_empty_packages
                const auto &sp = dynamic_cast<package &>(*subpackage);
                if (!sp.is_empty()) {
                    together_group_stack_.enter();

                    generate(sp, ostr);

                    together_group_stack_.leave();
                }
            }
            else {
                generator_.model().dynamic_apply(
                    subpackage.get(), [&](auto *el) {
                        auto together_group =
                            generator_.config().get_together_group(
                                el->full_name(false));
                        if (together_group) {
                            together_group_stack_.group_together(
                                together_group.value(), el);
                        }
                        else {
                            generator_.generate_alias(*el, ostr);
                            generator_.generate(*el, ostr);
                        }
                    });
            }

            if (!together_group_stack_.is_flat()) {
                generate_groups(ostr);
            }
        }

        generator_.end_package(p, ostr);
    }

    void generate_top_level_elements(std::ostream &ostr) const
    {
        for (const auto &p : generator_.model()) {
            if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
                if (!pkg->is_empty())
                    generate(*pkg, ostr);
            }
            else {
                generator_.model().dynamic_apply(p.get(), [&](auto *el) {
                    auto together_group =
                        generator_.config().get_together_group(
                            el->full_name(false));
                    if (together_group) {
                        together_group_stack_.group_together(
                            together_group.value(), el);
                    }
                    else {
                        generator_.generate_alias(*el, ostr);
                        generator_.generate(*el, ostr);
                    }
                });
            }
        }
    }

    void generate_relationships(const package &p, std::ostream &ostr) const
    {
        for (const auto &subpackage : p) {
            if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
                // TODO: add option - generate_empty_packages, currently
                //       packages which do not contain anything but other
                //       packages are skipped
                const auto &sp = dynamic_cast<package &>(*subpackage);
                if (!sp.is_empty())
                    generate_relationships(sp, ostr);
            }
            else {
                generator_.model().dynamic_apply(
                    subpackage.get(), [&](auto *el) {
                        if (generator_.model().should_include(*el)) {
                            generator_.generate_relationships(*el, ostr);
                        }
                    });
            }
        }
    }

    void generate_relationships(std::ostream &ostr) const
    {
        for (const auto &p : generator_.model()) {
            if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
                generate_relationships(*pkg, ostr);
            }
            else {
                generator_.model().dynamic_apply(p.get(), [&](auto *el) {
                    generator_.generate_relationships(*el, ostr);
                });
            }
        }
    }

    void generate_groups(std::ostream &ostr) const
    {
        for (const auto &[group_name, group_elements] :
            together_group_stack_.get_current_groups()) {

            generator_.start_together_group(group_name, ostr);

            for (auto *e : group_elements) {
                generator_.model().dynamic_apply(e, [&](auto *el) {
                    generator_.generate_alias(*el, ostr);
                    generator_.generate(*el, ostr);
                });
            }

            generator_.end_together_group(group_name, ostr);
        }
    }

    template <typename T>
    void sort_class_elements(std::vector<T> &elements) const
    {
        if (generator_.config().member_order() ==
            config::member_order_t::lexical) {
            std::sort(elements.begin(), elements.end(),
                [](const auto &m1, const auto &m2) {
                    return m1.name() < m2.name();
                });
        }
    }

    void generate_method_groups(
        const method_groups_t &methods, std::ostream &ostr) const
    {
        bool is_first_non_empty_group{true};

        for (const auto &group : method_groups_) {
            const auto &group_methods = methods.at(group);
            if (!group_methods.empty()) {
                if (!is_first_non_empty_group)
                    generator_.start_method_group(ostr);
                is_first_non_empty_group = false;
                generator_.generate_methods(group_methods, ostr);
            }
        }
    }

    /**
     * @brief Group class methods based on method type.
     *
     * @param methods List of class methods.
     *
     * @return Map of method groups.
     */
    method_groups_t group_methods(
        const std::vector<model::class_method> &methods) const
    {
        std::map<std::string, std::vector<model::class_method>> result;

        for (const auto &g : method_groups_) {
            result[g] = {};
        }

        for (const auto &m : methods) {
            if (m.is_constructor() || m.is_destructor()) {
                result["constructors"].push_back(m);
            }
            else if (m.is_copy_assignment() || m.is_move_assignment()) {
                result["assignment"].push_back(m);
            }
            else if (m.is_operator()) {
                result["operators"].push_back(m);
            }
            else {
                result["other"].push_back(m);
            }
        }

        return result;
    }

private:
    const std::vector<std::string> method_groups_{
        "constructors", "assignment", "operators", "other"};

    G &generator_;
    mutable common::generators::nested_element_stack<common::model::element>
        together_group_stack_;
};

} // namespace clanguml::class_diagram::generators