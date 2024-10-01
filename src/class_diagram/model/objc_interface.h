/**
 * @file src/class_diagram/model/objc_interface.h
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
#include "common/model/stylable_element.h"
#include "objc_member.h"
#include "objc_method.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

/*
 * @brief Diagram element representing an ObjC interface.
 */
class objc_interface : public common::model::element,
                       public common::model::stylable_element {
public:
    objc_interface(const common::model::namespace_ &using_namespaces);

    objc_interface(const objc_interface &) = delete;
    objc_interface(objc_interface &&) = delete;
    objc_interface &operator=(const objc_interface &) = delete;
    objc_interface &operator=(objc_interface &&) = delete;

    std::string type_name() const override
    {
        if (is_protocol())
            return "objc_protocol";

        if (is_category())
            return "objc_category";

        return "objc_interface";
    }

    friend bool operator==(const objc_interface &l, const objc_interface &r);

    std::string full_name(bool relative = true) const override;

    void add_member(objc_member &&member);

    void add_method(objc_method &&method);

    const std::vector<objc_member> &members() const;

    const std::vector<objc_method> &methods() const;

    bool is_protocol() const;

    void is_protocol(bool ip);

    bool is_category() const;

    void is_category(bool cat);

    /**
     * @brief Get Doxygen link to documentation page for this element.
     *
     * @return Doxygen link for this element.
     */
    std::optional<std::string> doxygen_link() const override;

private:
    std::vector<objc_member> members_;
    std::vector<objc_method> methods_;
    bool is_protocol_{false};
    bool is_category_{false};
};

} // namespace clanguml::class_diagram::model
