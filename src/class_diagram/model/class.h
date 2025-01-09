/**
 * @file src/class_diagram/model/class.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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
#include "common/model/enums.h"
#include "common/model/stylable_element.h"
#include "common/model/template_element.h"
#include "common/model/template_parameter.h"
#include "common/model/template_trait.h"
#include "common/types.h"

#include <string>
#include <vector>

namespace clanguml::common::model {
class diagram_filter;
}

namespace clanguml::class_diagram::model {

/**
 * @brief Diagram element representing a class or class template.
 */
class class_ : public common::model::template_element,
               public common::model::stylable_element {
public:
    class_(const common::model::namespace_ &using_namespace);

    class_(const class_ &) = delete;
    class_(class_ &&) noexcept = delete;
    class_ &operator=(const class_ &) = delete;
    class_ &operator=(class_ &&) = delete;

    friend bool operator==(const class_ &l, const class_ &r);

    /**
     * Get the type name of the diagram element.
     *
     * @return Type name of the diagram element.
     */
    std::string type_name() const override { return "class"; }

    /**
     * Whether or not the class was declared in the code as 'struct'.
     *
     * @return True, if the class was declared as 'struct'
     */
    bool is_struct() const;

    /**
     * Set, whether the class was declared as 'struct'.
     *
     * @param is_struct True, if the class was declared as 'struct'
     */
    void is_struct(bool is_struct);

    /**
     * Whether or not the class is a union.
     *
     * @return True, if the class is a union.
     */
    bool is_union() const;

    /**
     * Set, whether the class is a union.
     *
     * @param u True, if the class is a union.
     */
    void is_union(bool is_union);

    /**
     * Add a data member to the class.
     *
     * @param member Class data member.
     */
    void add_member(class_member &&member);

    /**
     * Add a method to the class.
     *
     * @param method Class method.
     */
    void add_method(class_method &&method);

    /**
     * Get reference to class member list.
     *
     * @return Reference to class members.
     */
    const std::vector<class_member> &members() const;

    /**
     * Get reference to class method list.
     *
     * @return Reference to class methods.
     */
    const std::vector<class_method> &methods() const;

    /**
     * @brief Get unqualified class ful name.
     *
     * This method returns the class full name but without any namespace
     * qualifier.
     *
     * @return Full class name without namespace.
     */
    std::string full_name_no_ns() const override;

    /**
     * Whether the class is abstract.
     *
     * @return True, if at least one method is abstract (=0).
     */
    bool is_abstract() const;

    /**
     * @brief Generate Doxygen style HTML link for the class.
     *
     * This method generates a link, which can be used in SVG diagrams to
     * create links from classes to Doxygen documentation pages.
     *
     * @return Doxygen-style HTML link for the class.
     */
    std::optional<std::string> doxygen_link() const override;

    void apply_filter(const common::model::diagram_filter &filter,
        const std::set<common::model::eid_t> &removed) override;

protected:
    /**
     * @brief Get class full name.
     *
     * This method renders the entire class name including all template
     * parameters and/or arguments.
     *
     * @param relative Whether the class name should be relative to
     *                 using_namespace
     * @return Full class name.
     */
    std::string full_name_impl(bool relative = true) const override;

private:
    bool is_struct_{false};
    bool is_union_{false};
    std::vector<class_member> members_;
    std::vector<class_method> methods_;
    std::string full_name_;
};

} // namespace clanguml::class_diagram::model

namespace std {
template <>
struct hash<std::reference_wrapper<clanguml::class_diagram::model::class_>> {
    std::size_t operator()(
        const std::reference_wrapper<clanguml::class_diagram::model::class_>
            &key) const
    {
        return std::hash<uint64_t>{}(key.get().id().value());
    }
};
} // namespace std
