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

#include "class_diagram/model/diagram.h"
//#include "class_diagram/visitor/translation_unit_context.h"
#include "common/model/enums.h"
#include "config/config.h"

//#include <clang-c/CXCompilationDatabase.h>
//#include <clang-c/Index.h>
//#include <cppast/cpp_friend.hpp>
//#include <cppast/cpp_function_template.hpp>
//#include <cppast/cpp_member_function.hpp>
//#include <cppast/cpp_member_variable.hpp>
//#include <cppast/cpp_template.hpp>
//#include <cppast/cpp_template_parameter.hpp>
//#include <cppast/cpp_type.hpp>
//#include <cppast/visitor.hpp>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <class_diagram/model/class.h>
#include <common/model/enums.h>
#include <type_safe/reference.hpp>
//#include <cppast/cpp_alias_template.hpp>
//#include <cppast/cpp_type_alias.hpp>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::class_diagram::visitor {

using found_relationships_t =
    std::vector<std::pair<clanguml::common::model::element::id_t,
        common::model::relationship_t>>;

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor> {
public:
    explicit translation_unit_visitor(clang::SourceManager &sm,
        clanguml::class_diagram::model::diagram &diagram,
        const clanguml::config::class_diagram &config);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *d);

    virtual bool VisitEnumDecl(clang::EnumDecl *e);

    //    virtual bool VisitVarDecl(clang::VarDecl *variable_declaration);
    clanguml::class_diagram::model::diagram &diagram() { return diagram_; }
    //    void operator()();

private:
    void process_class_bases(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c) const;

    void process_class_children(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c);

    void process_record_containment(const clang::TagDecl &record,
        clanguml::common::model::element &c) const;

    void process_method(const clang::CXXMethodDecl &mf,
        clanguml::class_diagram::model::class_ &c);

    void process_static_field(const clang::VarDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    void process_field(const clang::FieldDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    void process_function_parameter(const clang::ParmVarDecl &param,
        clanguml::class_diagram::model::class_method &method,
        clanguml::class_diagram::model::class_ &c,
        const std::set<std::string> &template_parameter_names = {});

    bool find_relationships(const clang::QualType &type, &,
        clanguml::common::model::relationship_t relationship_hint);

    void add_relationships(clanguml::class_diagram::model::class_ &c,
        const found_relationships_t &relationships,
        clanguml::common::model::access_t access,
        std::optional<std::string> label);

    void set_source_location(const clang::Decl &decl,
        clanguml::common::model::source_location &element);

    template <typename ClangDecl>
    void process_comment(
        const ClangDecl &decl, clanguml::common::model::decorated_element &e)
    {
        const auto *comment =
            decl.getASTContext().getRawCommentForDeclNoCache(&decl);

        if (comment != nullptr) {
            e.set_comment(comment->getBriefText(decl.getASTContext()));

            //            if (mf.location().has_value()) {
            //                e.set_file(mf.location().value().file);
            //                e.set_line(mf.location().value().line);
            //            }

            e.add_decorators(decorators::parse(e.comment().value()));
        }
    }

    clang::SourceManager &source_manager_;

    // Reference to the output diagram model
    clanguml::class_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::class_diagram &config_;
};

/*
class translation_unit_visitor {
public:
    translation_unit_visitor(cppast::cpp_entity_index &idx,
        clanguml::class_diagram::model::diagram &diagram,
        const clanguml::config::class_diagram &config);

    void operator()(const cppast::cpp_entity &file);

    void process_class_declaration(const cppast::cpp_class &cls,
        type_safe::optional_ref<const cppast::cpp_template_specialization>
            tspec = nullptr);

    void process_enum_declaration(const cppast::cpp_enum &enm);

    void process_anonymous_enum(const cppast::cpp_enum &en,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_field(const cppast::cpp_member_variable &mv,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    bool process_field_with_template_instantiation(
        const cppast::cpp_member_variable &mv, const cppast::cpp_type &type,
        clanguml::class_diagram::model::class_ &c,
        clanguml::class_diagram::model::class_member &member,
        cppast::cpp_access_specifier_kind as);

    void process_static_field(const cppast::cpp_variable &mv,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_method(const cppast::cpp_member_function &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_template_method(const cppast::cpp_function_template &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_static_method(const cppast::cpp_function &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_constructor(const cppast::cpp_constructor &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_destructor(const cppast::cpp_destructor &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_function_parameter(const cppast::cpp_function_parameter
&param, clanguml::class_diagram::model::class_method &m,
        clanguml::class_diagram::model::class_ &c,
        const std::set<std::string> &template_parameter_names = {});

    bool find_relationships(const cppast::cpp_type &t,
        found_relationships_t &relationships,
        clanguml::common::model::relationship_t relationship_hint =
            clanguml::common::model::relationship_t::kNone) const;

    void process_template_type_parameter(
        const cppast::cpp_template_type_parameter &t,
        clanguml::class_diagram::model::class_ &parent);

    void process_template_nontype_parameter(
        const cppast::cpp_non_type_template_parameter &t,
        clanguml::class_diagram::model::class_ &parent);

    void process_template_template_parameter(
        const cppast::cpp_template_template_parameter &t,
        clanguml::class_diagram::model::class_ &parent);

    void process_friend(const cppast::cpp_friend &t,
        clanguml::class_diagram::model::class_ &parent,
        cppast::cpp_access_specifier_kind as);

    void process_namespace(const cppast::cpp_entity &e,
        const cppast::cpp_namespace &ns_declaration);

    void process_type_alias(const cppast::cpp_type_alias &ta);

    void process_type_alias_template(const cppast::cpp_alias_template &at);

    void process_class_children(const cppast::cpp_class &cls, model::class_
&c);

    void process_class_bases(
        const cppast::cpp_class &cls, model::class_ &c) const;

    void process_unexposed_template_specialization_parameters(
        const type_safe::optional_ref<const
cppast::cpp_template_specialization> &tspec, model::class_ &c) const;

    void process_exposed_template_specialization_parameters(
        const type_safe::optional_ref<const
cppast::cpp_template_specialization> &tspec, model::class_ &c);

    void process_scope_template_parameters(
        model::class_ &c, const cppast::cpp_scope_name &scope);

    bool process_template_parameters(const cppast::cpp_class &cls,
        model::class_ &c,
        const type_safe::optional_ref<const
cppast::cpp_template_specialization> &tspec);

    void process_class_containment(
        const cppast::cpp_class &cls, model::class_ &c) const;

private:
    std::unique_ptr<clanguml::class_diagram::model::class_>
    build_template_instantiation(
        const cppast::cpp_template_instantiation_type &t,
        std::optional<clanguml::class_diagram::model::class_ *> parent =
{});

    const cppast::cpp_type &resolve_alias(const cppast::cpp_type &t);

    const cppast::cpp_type &resolve_alias_template(
        const cppast::cpp_type &type);

    bool find_relationships_in_array(
        found_relationships_t &relationships, const cppast::cpp_type &t)
const;

    bool find_relationships_in_pointer(const cppast::cpp_type &t_,
        found_relationships_t &relationships,
        const common::model::relationship_t &relationship_hint) const;

    bool find_relationships_in_reference(const cppast::cpp_type &t_,
        found_relationships_t &relationships,
        const common::model::relationship_t &relationship_hint) const;

    bool find_relationships_in_user_defined_type(const cppast::cpp_type &t_,
        found_relationships_t &relationships, const std::string &fn,
        common::model::relationship_t &relationship_type,
        const cppast::cpp_type &t) const;

    bool find_relationships_in_template_instantiation(const cppast::cpp_type
&t, const std::string &fn, found_relationships_t &relationships,
        common::model::relationship_t relationship_type) const;

    bool find_relationships_in_unexposed_template_params(
        const model::template_parameter &ct,
        found_relationships_t &relationships) const;

    void build_template_instantiation_primary_template(
        const cppast::cpp_template_instantiation_type &t,
        clanguml::class_diagram::model::class_ &tinst,
        std::deque<std::tuple<std::string, int, bool>>
&template_base_params, std::optional<clanguml::class_diagram::model::class_
*> &parent, std::string &full_template_name) const;

    void build_template_instantiation_process_type_argument(
        const std::optional<clanguml::class_diagram::model::class_ *>
&parent, model::class_ &tinst, const cppast::cpp_type &targ_type,
        class_diagram::model::template_parameter &ct);

    void build_template_instantiation_process_expression_argument(
        const cppast::cpp_template_argument &targ,
        model::template_parameter &ct) const;

    bool build_template_instantiation_add_base_classes(model::class_ &tinst,
        std::deque<std::tuple<std::string, int, bool>>
&template_base_params, int arg_index, bool variadic_params, const
model::template_parameter &ct) const;

    void process_function_parameter_find_relationships_in_template(
        model::class_ &c, const std::set<std::string>
&template_parameter_names, const cppast::cpp_type &t);

    // ctx allows to track current visitor context, e.g. current namespace
    translation_unit_context ctx;
    bool simplify_system_template(
        model::template_parameter &parameter, const std::string
&basicString);

    bool add_nested_template_relationships(
        const cppast::cpp_member_variable &mv, model::class_ &c,
        model::class_member &m, cppast::cpp_access_specifier_kind &as,
        const model::class_ &tinst,
        common::model::relationship_t &relationship_type,
        common::model::relationship_t &decorator_rtype,
        std::string &decorator_rmult);
};
 */
}
