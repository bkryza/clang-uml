/**
 * @file src/common/model/element.h
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

#include "diagram_element.h"
#include "namespace.h"
#include "relationship.h"
#include "source_location.h"
#include "util/util.h"

#include <inja/inja.hpp>

#include <atomic>
#include <exception>
#include <string>
#include <vector>

namespace clanguml::common::model {

/**
 * @brief Base class for any element qualified by namespace.
 */
class element : public diagram_element {
public:
    element(namespace_ using_namespace, path_type pt = path_type::kNamespace);

    ~element() override = default;

    /**
     * Return the elements fully qualified name, but without template
     * arguments or function params.
     *
     * @return Fully qualified element name.
     */
    std::string name_and_ns() const
    {
        auto ns = ns_ | name();
        return ns.to_string();
    }

    /**
     * Set elements namespace.
     *
     * @param ns Namespace.
     */
    void set_namespace(const namespace_ &ns) { ns_ = ns; }

    /**
     * Return elements namespace.
     *
     * @return Namespace.
     */
    namespace_ get_namespace() const { return ns_; }

    /**
     * Return elements relative namespace.
     *
     * @return Namespace.
     */
    namespace_ get_relative_namespace() const
    {
        return ns_.relative_to(using_namespace_);
    }

    /**
     * Return elements namespace as path.
     *
     * Namespace is a nested path in diagrams where packages are generated
     * from namespaces.
     *
     * @return Namespace.
     */
    const namespace_ &path() const { return ns_; }

    /**
     * Set elements owning module.
     *
     * @param module C++20 module.
     */
    void set_module(const std::string &module) { module_ = module; }

    /**
     * Return elements owning module, if any.
     *
     * @return C++20 module.
     */
    std::optional<std::string> module() const { return module_; }

    /**
     * Set whether the element is in a private module
     *
     * @param module C++20 module.
     */
    void set_module_private(const bool module_private)
    {
        module_private_ = module_private;
    }

    /**
     * Check whether the element is in a private module.
     *
     * @return C++20 module.
     */
    bool module_private() const { return module_private_; }

    /**
     * Return elements full name.
     *
     * @return Fully qualified elements name.
     */
    std::string full_name(bool relative) const override
    {
        if (relative)
            return name();

        return name_and_ns();
    }

    /**
     * Return elements full name but without namespace.
     *
     * @return Elements full name without namespace.
     */
    virtual std::string full_name_no_ns() const { return name(); }

    /**
     * Return the relative namespace from config.
     *
     * @return Namespace.
     */
    const namespace_ &using_namespace() const;

    friend bool operator==(const element &l, const element &r);

    friend std::ostream &operator<<(std::ostream &out, const element &rhs);

    inja::json context() const override;

private:
    namespace_ ns_;
    namespace_ using_namespace_;
    std::optional<std::string> module_;
    bool module_private_{false};
};
} // namespace clanguml::common::model
