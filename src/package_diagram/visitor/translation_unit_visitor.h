/**
 * src/package_diagram/visitor/translation_unit_visitor.h
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

#include "config/config.h"
#include "package_diagram/model/diagram.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <common/model/enums.h>
#include <common/model/package.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::package_diagram::visitor {

using found_relationships_t =
    std::vector<std::pair<clanguml::common::model::diagram_element::id_t,
        common::model::relationship_t>>;

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor> {
public:
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::package_diagram::model::diagram &diagram,
        const clanguml::config::package_diagram &config);

    virtual bool VisitNamespaceDecl(clang::NamespaceDecl *ns);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *cls);

    virtual bool VisitFunctionDecl(clang::FunctionDecl *function_declaration);

    clanguml::package_diagram::model::diagram &diagram() { return diagram_; }

    const clanguml::config::package_diagram &config() const { return config_; }

    void finalize() { }

private:
    void process_class_declaration(
        const clang::CXXRecordDecl &cls, found_relationships_t &relationships);

    void process_class_children(
        const clang::CXXRecordDecl &cls, found_relationships_t &relationships);

    void process_class_bases(
        const clang::CXXRecordDecl &cls, found_relationships_t &relationships);

    void process_method(const clang::CXXMethodDecl &method,
        found_relationships_t &relationships);

    void process_template_method(const clang::FunctionTemplateDecl &method,
        found_relationships_t &relationships);

    void process_field(const clang::FieldDecl &field_declaration,
        found_relationships_t &relationships);

    void process_static_field(const clang::VarDecl &field_declaration,
        found_relationships_t &relationships);

    void process_friend(const clang::FriendDecl &friend_declaration,
        found_relationships_t &relationships);

    bool find_relationships(const clang::QualType &type,
        found_relationships_t &relationships,
        common::model::relationship_t relationship_hint =
            common::model::relationship_t::kDependency);

    void add_relationships(
        clang::DeclContext *cls, found_relationships_t &relationships);

    template <typename ClangDecl>
    void process_comment(
        const ClangDecl &decl, clanguml::common::model::decorated_element &e)
    {
        const auto *comment =
            decl.getASTContext().getRawCommentForDeclNoCache(&decl);

        if (comment != nullptr) {
            e.set_comment(comment->getFormattedText(
                source_manager_, decl.getASTContext().getDiagnostics()));
            e.add_decorators(decorators::parse(e.comment().value()));
        }
    }

    void set_source_location(const clang::Decl &decl,
        clanguml::common::model::source_location &element)
    {
        if (decl.getLocation().isValid()) {
            element.set_file(
                source_manager_.getFilename(decl.getLocation()).str());
            element.set_line(
                source_manager_.getSpellingLineNumber(decl.getLocation()));
        }
    }

    clang::SourceManager &source_manager_;

    // Reference to the output diagram model
    clanguml::package_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::package_diagram &config_;
};
}
