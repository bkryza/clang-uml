/**
 * src/sequence_diagram/model/participant.h
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

#include "class_diagram/model/template_parameter.h"
#include "common/model/element.h"

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

struct template_trait {
    std::ostringstream &render_template_params(std::ostringstream &ostr,
        const common::model::namespace_ &using_namespace, bool relative) const;

    void set_base_template(const std::string &full_name);

    std::string base_template() const;

    void add_template(class_diagram::model::template_parameter tmplt);

    const std::vector<class_diagram::model::template_parameter> &
    templates() const;

    int calculate_template_specialization_match(
        const template_trait &other, const std::string &full_name) const;

    bool is_implicit() const;

    void set_implicit(bool implicit);

private:
    std::vector<class_diagram::model::template_parameter> templates_;
    std::string base_template_full_name_;
    bool is_implicit_{false};
};

struct participant : public common::model::element,
                     public common::model::stylable_element {
    enum class stereotype_t {
        participant = 0,
        actor,
        boundary,
        control,
        entity,
        database,
        collections,
        queue
    };

    using common::model::element::element;

    participant(const participant &) = delete;
    participant(participant &&) noexcept = delete;
    participant &operator=(const participant &) = delete;
    participant &operator=(participant &&) = delete;

    std::string type_name() const override { return "participant"; }

    virtual std::string to_string() const;

    stereotype_t stereotype_{stereotype_t::participant};
};

struct class_ : public participant, public template_trait {
public:
    class_(const common::model::namespace_ &using_namespace);

    class_(const class_ &) = delete;
    class_(class_ &&) noexcept = delete;
    class_ &operator=(const class_ &) = delete;
    class_ &operator=(class_ &&) = delete;

    std::string type_name() const override { return "class"; }

    bool is_struct() const;
    void is_struct(bool is_struct);

    bool is_template() const;
    void is_template(bool is_template);

    bool is_template_instantiation() const;
    void is_template_instantiation(bool is_template_instantiation);

    friend bool operator==(const class_ &l, const class_ &r);

    std::string full_name(bool relative = true) const override;

    std::string full_name_no_ns() const override;

    bool is_abstract() const;

    bool is_alias() const;

    void is_alias(bool alias);

    bool is_lambda() const;

    void is_lambda(bool is_lambda);

private:
    bool is_struct_{false};
    bool is_template_{false};
    bool is_template_instantiation_{false};
    bool is_alias_{false};
    bool is_lambda_{false};

    std::string full_name_;
};

struct lambda : public class_ {
    using class_::class_;

    std::string type_name() const override { return "lambda"; }
};

struct function : public participant {
    enum class message_render_mode { full, abbreviated, no_arguments };

    function(const common::model::namespace_ &using_namespace);

    function(const function &) = delete;
    function(function &&) noexcept = delete;
    function &operator=(const function &) = delete;
    function &operator=(function &&) = delete;

    std::string type_name() const override { return "function"; }

    std::string full_name(bool relative = true) const override;

    std::string full_name_no_ns() const override;

    virtual std::string message_name(message_render_mode mode) const;

    bool is_const() const;

    void is_const(bool c);

    bool is_void() const;

    void is_void(bool v);

    bool is_static() const;

    void is_static(bool s);

    void add_parameter(const std::string &a);

    const std::vector<std::string> &parameters() const;

private:
    bool is_const_{false};
    bool is_void_{false};
    bool is_static_{false};
    std::vector<std::string> parameters_;
};

struct method : public function {
    method(const common::model::namespace_ &using_namespace);

    method(const function &) = delete;
    method(method &&) noexcept = delete;
    method &operator=(const method &) = delete;
    method &operator=(method &&) = delete;

    std::string type_name() const override { return "method"; }

    std::string method_name() const;

    std::string alias() const override;

    void set_method_name(const std::string &name);

    void set_class_id(diagram_element::id_t id);

    void set_class_full_name(const std::string &name);

    const auto &class_full_name() const;

    std::string full_name(bool /*relative*/) const override;

    std::string message_name(message_render_mode mode) const override;

    diagram_element::id_t class_id() const;

    std::string to_string() const override;

private:
    diagram_element::id_t class_id_{};
    std::string method_name_;
    std::string class_full_name_;
};

struct function_template : public function, public template_trait {
    function_template(const common::model::namespace_ &using_namespace);

    function_template(const function_template &) = delete;
    function_template(function_template &&) noexcept = delete;
    function_template &operator=(const function_template &) = delete;
    function_template &operator=(function_template &&) = delete;

    std::string type_name() const override { return "function_template"; }

    std::string full_name(bool relative = true) const override;

    std::string full_name_no_ns() const override;

    std::string message_name(message_render_mode mode) const override;
};
} // namespace clanguml::sequence_diagram::model
