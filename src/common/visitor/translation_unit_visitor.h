/**
 * src/common/visitor/translation_unit_visitor.h
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

#include "comment/comment_visitor.h"
#include "config/config.h"

#include <clang/AST/Comment.h>
#include <clang/Basic/SourceManager.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::common::visitor {

using found_relationships_t =
    std::vector<std::pair<clanguml::common::model::diagram_element::id_t,
        common::model::relationship_t>>;

class translation_unit_visitor {
public:
    explicit translation_unit_visitor(
        clang::SourceManager &sm, const clanguml::config::diagram &config);

    clang::SourceManager &source_manager() const;

    void finalize();

protected:
    void set_source_location(const clang::Decl &decl,
        clanguml::common::model::source_location &element);

    void process_comment(const clang::NamedDecl &decl,
        clanguml::common::model::decorated_element &e);

private:
    clang::SourceManager &source_manager_;

    // Reference to diagram config
    const clanguml::config::diagram &config_;

    std::unique_ptr<comment::comment_visitor> comment_visitor_;
};
}
