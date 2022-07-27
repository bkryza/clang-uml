/**
 * src/class_diagram/visitor/translation_unit_visitor.h
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

#include "class_diagram/model/class.h"
#include "class_diagram/model/diagram.h"
#include "common/model/enums.h"
#include "config/config.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::class_diagram::visitor {

using found_relationships_t =
    std::vector<std::pair<clanguml::common::model::diagram_element::id_t,
        common::model::relationship_t>>;

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor> {
public:
    explicit translation_unit_visitor(clang::SourceManager &sm,
        clanguml::class_diagram::model::diagram &diagram,
        const clanguml::config::class_diagram &config);

    virtual bool VisitNamespaceDecl(clang::NamespaceDecl *ns);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *d);

    virtual bool VisitEnumDecl(clang::EnumDecl *e);

    virtual bool VisitClassTemplateDecl(
        clang::ClassTemplateDecl *class_template_declaration);

    virtual bool VisitClassTemplateSpecializationDecl(
        clang::ClassTemplateSpecializationDecl *cls);

    virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *cls);

    //    virtual bool VisitVarDecl(clang::VarDecl *variable_declaration);
    clanguml::class_diagram::model::diagram &diagram() { return diagram_; }

    const clanguml::config::class_diagram &config() const { return config_; }

    //    void operator()();

    void finalize();

private:
    std::unique_ptr<clanguml::class_diagram::model::class_>
    create_class_declaration(clang::CXXRecordDecl *cls);

    void process_class_declaration(const clang::CXXRecordDecl &cls,
        clanguml::class_diagram::model::class_ &c);

    std::unique_ptr<clanguml::class_diagram::model::class_>
    process_template_specialization(
        clang::ClassTemplateSpecializationDecl *cls);

    void process_class_bases(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c) const;

    void process_class_children(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c);

    void process_template_specialization_children(
        const clang::ClassTemplateSpecializationDecl *cls,
        clanguml::class_diagram::model::class_ &c);

    bool process_template_parameters(
        const clang::ClassTemplateDecl &template_declaration,
        clanguml::class_diagram::model::class_ &c);

    void process_record_containment(const clang::TagDecl &record,
        clanguml::common::model::element &c) const;

    void process_method(const clang::CXXMethodDecl &mf,
        clanguml::class_diagram::model::class_ &c);

    void process_template_method(const clang::FunctionTemplateDecl &mf,
        clanguml::class_diagram::model::class_ &c);

    void process_static_field(const clang::VarDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    void process_field(const clang::FieldDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    void process_function_parameter(const clang::ParmVarDecl &param,
        clanguml::class_diagram::model::class_method &method,
        clanguml::class_diagram::model::class_ &c,
        const std::set<std::string> &template_parameter_names = {});

    void process_friend(
        const clang::FriendDecl &f, clanguml::class_diagram::model::class_ &c);

    bool find_relationships(const clang::QualType &type,
        found_relationships_t &,
        clanguml::common::model::relationship_t relationship_hint);

    void add_relationships(clanguml::class_diagram::model::class_ &c,
        const clanguml::class_diagram::model::class_member &field,
        const found_relationships_t &relationships);

    void set_source_location(const clang::Decl &decl,
        clanguml::common::model::source_location &element);

    std::unique_ptr<clanguml::class_diagram::model::class_>
    build_template_instantiation(
        const clang::TemplateSpecializationType &template_type,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    void process_function_parameter_find_relationships_in_template(
        clanguml::class_diagram::model::class_ &c,
        const std::set<std::string> &template_parameter_names,
        const clang::TemplateSpecializationType &template_instantiation_type);

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

    void add_incomplete_forward_declarations();

    bool simplify_system_template(
        model::template_parameter &ct, const std::string &full_name);

    clang::SourceManager &source_manager_;

    // Reference to the output diagram model
    clanguml::class_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::class_diagram &config_;

    std::map<common::model::diagram_element::id_t,
        std::unique_ptr<clanguml::class_diagram::model::class_>>
        forward_declarations_;
};
}
