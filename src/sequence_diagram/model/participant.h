/**
 * src/sequence_diagram/model/participant.h
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

#include "class_diagram/model/template_parameter.h"
#include "class_diagram/model/type_alias.h"
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

    bool is_implicit() const { return is_implicit_; }

    void set_implicit(bool implicit) { is_implicit_ = implicit; }

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

    virtual std::string to_string() const
    {
        return fmt::format("Participant '{}': id={} name={}", type_name(), id(),
            full_name(false));
    }

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

    bool is_alias() const { return is_alias_; }

    void is_alias(bool alias) { is_alias_ = alias; }

    bool is_lambda() const { return is_lambda_; }

    void is_lambda(bool is_lambda) { is_lambda_ = is_lambda; }

private:
    bool is_struct_{false};
    bool is_template_{false};
    bool is_template_instantiation_{false};
    bool is_alias_{false};
    bool is_lambda_{false};

    std::map<std::string, clanguml::class_diagram::model::type_alias>
        type_aliases_;

    std::string full_name_;
};

struct lambda : public class_ {
    using class_::class_;

    std::string type_name() const override { return "lambda"; }
};

struct function : public participant {
    function(const common::model::namespace_ &using_namespace);

    function(const function &) = delete;
    function(function &&) noexcept = delete;
    function &operator=(const function &) = delete;
    function &operator=(function &&) = delete;

    std::string type_name() const override { return "function"; }

    std::string full_name(bool relative = true) const override;

    std::string full_name_no_ns() const override;
};

struct method : public participant {
    method(const common::model::namespace_ &using_namespace);

    method(const function &) = delete;
    method(method &&) noexcept = delete;
    method &operator=(const method &) = delete;
    method &operator=(method &&) = delete;

    std::string type_name() const override { return "method"; }

    const std::string method_name() const { return method_name_; }

    std::string alias() const override;

    void set_method_name(const std::string &name) { method_name_ = name; }

    void set_class_id(diagram_element::id_t id) { class_id_ = id; }

    void set_class_full_name(const std::string &name)
    {
        class_full_name_ = name;
    }

    const auto &class_full_name() const { return class_full_name_; }

    std::string full_name(bool /*relative*/) const override
    {
        return class_full_name() + "::" + method_name();
    }

    diagram_element::id_t class_id() const { return class_id_; }

    std::string to_string() const override
    {
        return fmt::format("Participant '{}': id={}, name={}, class_id={}",
            type_name(), id(), full_name(false), class_id());
    }

private:
    diagram_element::id_t class_id_;
    std::string method_name_;
    std::string class_full_name_;
};

struct function_template : public participant, public template_trait {
    function_template(const common::model::namespace_ &using_namespace);

    function_template(const function_template &) = delete;
    function_template(function_template &&) noexcept = delete;
    function_template &operator=(const function_template &) = delete;
    function_template &operator=(function_template &&) = delete;

    std::string type_name() const override { return "function_template"; }

    std::string full_name(bool relative = true) const override;

    std::string full_name_no_ns() const override;
};

}
