/**
 * src/class_diagram/model/class.h
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

#include "class_member.h"
#include "class_method.h"
#include "class_parent.h"
#include "class_template.h"
#include "common/model/element.h"
#include "common/model/enums.h"
#include "common/model/stylable_element.h"
#include "type_alias.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

class class_ : public common::model::element,
               public common::model::stylable_element {
public:
    class_(const std::vector<std::string> &using_namespaces);

    class_(const class_ &) = delete;
    class_(class_ &&) noexcept = delete;
    class_ &operator=(const class_ &) = delete;
    class_ &operator=(class_ &&) = delete;

    bool is_struct() const;
    void is_struct(bool is_struct);

    bool is_template() const;
    void is_template(bool is_template);

    bool is_template_instantiation() const;
    void is_template_instantiation(bool is_template_instantiation);

    void add_member(class_member &&member);
    void add_method(class_method &&method);
    void add_parent(class_parent &&parent);
    void add_template(class_template &&tmplt);

    const std::vector<class_member> &members() const;
    const std::vector<class_method> &methods() const;
    const std::vector<class_parent> &parents() const;
    const std::vector<class_template> &templates() const;

    void set_base_template(const std::string &full_name);
    std::string base_template() const;

    friend bool operator==(const class_ &l, const class_ &r);

    void add_type_alias(type_alias &&ta);

    std::string full_name(bool relative = true) const override;

    bool is_abstract() const;

private:
    bool is_struct_{false};
    bool is_template_{false};
    bool is_template_instantiation_{false};
    std::vector<class_member> members_;
    std::vector<class_method> methods_;
    std::vector<class_parent> bases_;
    std::vector<class_template> templates_;
    std::string base_template_full_name_;
    std::map<std::string, type_alias> type_aliases_;

    std::string full_name_;
};

}
