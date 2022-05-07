/**
 * src/class_diagram/model/class_template.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include "common/model/namespace.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

class class_template {
public:
    class_template(const std::string &type = "", const std::string &name = "",
        const std::string &default_value = "", bool is_variadic = false);

    void set_type(const std::string &type);
    std::string type() const;

    void set_name(const std::string &name);
    std::string name() const;

    void set_default_value(const std::string &value);
    std::string default_value() const;

    void is_variadic(bool is_variadic) noexcept;
    bool is_variadic() const noexcept;

    bool is_specialization_of(const class_template &ct) const;

    friend bool operator==(const class_template &l, const class_template &r);
    friend bool operator!=(const class_template &l, const class_template &r);

    bool is_template_parameter() const { return is_template_parameter_; }

    void is_template_parameter(bool is_template_parameter)
    {
        is_template_parameter_ = is_template_parameter;
    }

    bool is_template_template_parameter() const
    {
        return is_template_template_parameter_;
    }

    void is_template_template_parameter(bool is_template_template_parameter)
    {
        is_template_template_parameter_ = is_template_template_parameter;
    }

    std::string to_string(
        const clanguml::common::model::namespace_ &using_namespace) const;

    std::vector<class_template> template_params_;

private:
    /// Represents the type of non-type template parameters
    /// e.g. 'int'
    std::string type_;

    /// The name of the parameter (e.g. 'T' or 'N')
    std::string name_;

    /// Default value of the template parameter
    std::string default_value_;

    /// Whether the template parameter is a regular template parameter
    /// When false, it is a non-type template parameter
    bool is_template_parameter_{false};

    /// Whether the template parameter is a template template parameter
    /// Can only be true when is_template_parameter_ is true
    bool is_template_template_parameter_{false};

    /// Whether the template parameter is variadic
    bool is_variadic_{false};
};
}
