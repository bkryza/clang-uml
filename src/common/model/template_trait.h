/**
 * src/common/model/template_trait.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

class template_trait {
public:
    std::ostream &render_template_params(std::ostream &ostr,
        const common::model::namespace_ &using_namespace, bool relative) const;

    void set_base_template(const std::string &full_name);

    std::string base_template() const;

    void add_template(template_parameter &&tmplt);

    const std::vector<template_parameter> &templates() const;

    int calculate_template_specialization_match(
        const template_trait &other) const;

    bool is_implicit() const;

    void set_implicit(bool implicit);

private:
    std::vector<template_parameter> templates_;
    std::string base_template_full_name_;
    bool is_implicit_{false};
};

} // namespace clanguml::common::model
