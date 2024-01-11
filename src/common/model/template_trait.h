/**
 * @file src/common/model/template_trait.h
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

#include "common/model/element.h"
#include "common/model/template_parameter.h"

#include <string>
#include <vector>

namespace clanguml::common::model {

/**
 * @brief Common interface for template diagram elements.
 *
 * @embed{template_trait_hierarchy_class.svg}
 */
class template_trait {
public:
    /**
     * Render the template parameters to a stream.
     *
     * @param ostr Output stream
     * @param using_namespace Relative to namespace
     * @param relative Whether to make template arguments relative to
     *                 `using_namespace`
     * @return Reference to output stream
     */
    std::ostream &render_template_params(std::ostream &ostr,
        const common::model::namespace_ &using_namespace, bool relative) const;

    /**
     * Add template parameter
     *
     * @param tmplt Template parameter
     */
    void add_template(template_parameter &&tmplt);

    /**
     * Get reference to template parameters.
     *
     * @return Reference to template parameters list.
     */
    const std::vector<template_parameter> &template_params() const;

    /**
     * @brief Wrapper around @ref
     * calculate_template_params_specialization_match()
     *
     * @param other Other template diagram element
     * @return Match value
     */
    int calculate_template_specialization_match(
        const template_trait &other) const;

private:
    std::vector<template_parameter> templates_;
    std::string base_template_full_name_;
};

} // namespace clanguml::common::model
