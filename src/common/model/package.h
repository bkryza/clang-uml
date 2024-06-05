/**
 * @file src/common/model/package.h
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
#include "common/model/nested_trait.h"
#include "common/model/path.h"
#include "common/model/stylable_element.h"
#include "common/types.h"
#include "util/util.h"

#include <spdlog/spdlog.h>

#include <set>
#include <string>
#include <vector>

namespace clanguml::common::model {

/**
 * @brief Diagram element representing namespace or directory package
 *
 * @embed{package_hierarchy_class.svg}
 */
class package : public element,
                public stylable_element,
                public nested_trait<element, path> {
public:
    package(const common::model::path &using_namespace,
        path_type pt = path_type::kNamespace);

    package(const package &) = delete;
    package(package &&) = default;
    package &operator=(const package &) = delete;
    package &operator=(package &&) = delete;

    std::string type_name() const override { return "package"; }

    std::string full_name(bool relative) const override;

    /**
     * Returns whether the namespace is deprecated.
     *
     * @return True, if namespace is deprecated.
     */
    bool is_deprecated() const;

    /**
     * Set namespace deprecation status.
     *
     * @param deprecated True, if namespace is deprecated
     */
    void set_deprecated(bool deprecated);

    /**
     * @brief Generate Doxygen style HTML link for the class.
     *
     * This method generates a link, which can be used in SVG diagrams to
     * create links from classes to Doxygen documentation pages.
     *
     * @return Doxygen-style HTML link for the class.
     */
    std::optional<std::string> doxygen_link() const override;

private:
    bool is_deprecated_{false};
};
} // namespace clanguml::common::model

namespace std {
template <>
struct hash<std::reference_wrapper<clanguml::common::model::package>> {
    std::size_t operator()(
        const std::reference_wrapper<clanguml::common::model::package> &key)
        const
    {
        using clanguml::common::eid_t;

        return key.get().id().value();
    }
};
} // namespace std