/**
 * @file src/common/model/filters/diagram_filter_factory.h
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

#include "diagram_filter.h"
#include "package_diagram/model/diagram.h"

namespace clanguml::common::model {
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

class diagram_filter_initializer {
public:
    diagram_filter_initializer(const config::diagram &c, diagram_filter &filter)
        : diagram_config{c}
        , df{filter}
    {
    }

    virtual ~diagram_filter_initializer() = default;

    virtual void initialize() = 0;

protected:
    const config::diagram &diagram_config;
    diagram_filter &df;
};

class basic_diagram_filter_initializer : public diagram_filter_initializer {
public:
    using diagram_filter_initializer::diagram_filter_initializer;

    ~basic_diagram_filter_initializer() override = default;

    void initialize() override;
};

class advanced_diagram_filter_initializer : public diagram_filter_initializer {
public:
    using diagram_filter_initializer::diagram_filter_initializer;

    ~advanced_diagram_filter_initializer() override = default;

    void initialize() override;

private:
    std::vector<std::unique_ptr<filter_visitor>> build(
        filter_t filter_type, const config::filter &filter_config);

    template <typename FT, typename T, typename... Args>
    void add_filter(const filter_t &filter_type,
        const std::vector<T> &filter_config,
        std::vector<std::unique_ptr<filter_visitor>> &result, Args &&...args)
    {
        if (!filter_config.empty()) {
            auto filter = std::make_unique<FT>(
                filter_type, filter_config, std::forward<Args>(args)...);
            filter->set_mode(filter_mode_t::advanced);
            result.emplace_back(std::move(filter));
        }
    }
};

template <>
void advanced_diagram_filter_initializer::add_filter<
    source_file_dependency_filter_t>(const filter_t &filter_type,
    const std::vector<common::string_or_regex> &filter_config,
    std::vector<std::unique_ptr<filter_visitor>> &result, relationship_t &&rt,
    bool &&direction);

class diagram_filter_factory {
public:
    static std::unique_ptr<diagram_filter> create(
        const common::model::diagram &d, const config::diagram &c)
    {
        auto filter = std::make_unique<diagram_filter>(
            d, c, diagram_filter::private_constructor_tag_t{});

        if (c.filter_mode() == config::filter_mode_t::basic) {
            basic_diagram_filter_initializer init{c, *filter};
            init.initialize();
        }
        else {
            advanced_diagram_filter_initializer init{c, *filter};
            init.initialize();
        }

        return filter;
    }
};

} // namespace clanguml::common::model