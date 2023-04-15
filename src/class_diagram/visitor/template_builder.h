/**
 * src/class_diagram/visitor/template_builder.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
#include "class_diagram/model/concept.h"
#include "class_diagram/model/diagram.h"
#include "common/visitor/ast_id_mapper.h"
#include "config/config.h"

namespace clanguml::class_diagram::visitor {

using class_diagram::model::class_;
using common::model::namespace_;
using common::model::relationship_t;
using common::model::template_parameter;

class template_builder {
public:
    template_builder(class_diagram::model::diagram &d,
        const config::class_diagram &config,
        common::visitor::ast_id_mapper &id_mapper,
        clang::SourceManager &source_manager);

    class_diagram::model::diagram &diagram();

    const config::class_diagram &config() const;

    const namespace_ &using_namespace() const;

    bool simplify_system_template(
        template_parameter &ct, const std::string &full_name) const;

    std::unique_ptr<clanguml::class_diagram::model::class_> build(
        const clang::Decl *cls,
        const clang::TemplateSpecializationType &template_type_decl,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    std::unique_ptr<clanguml::class_diagram::model::class_>
    build_from_class_template_specialization(
        const clang::ClassTemplateSpecializationDecl &template_specialization,
        const clang::RecordType &record_type,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    bool add_base_classes(clanguml::class_diagram::model::class_ &tinst,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        int arg_index, bool variadic_params,
        const clanguml::common::model::template_parameter &ct);

    void process_template_arguments(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::Decl *cls,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        const clang::ArrayRef<clang::TemplateArgument> &template_args,
        model::class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl);

    void argument_process_dispatch(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::Decl *cls, class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        std::vector<template_parameter> &argument);

    void process_tag_argument(model::class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        common::model::template_parameter &argument);

    template_parameter process_expression_argument(
        const clang::TemplateArgument &arg);

    template_parameter process_integral_argument(
        const clang::TemplateArgument &arg);

    template_parameter process_nullptr_argument(
        const clang::TemplateArgument &arg);

    template_parameter process_null_argument(
        const clang::TemplateArgument &arg);

    std::vector<template_parameter> process_pack_argument(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::Decl *cls, class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        std::vector<template_parameter> &argument);

    template_parameter process_type_argument(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::Decl *cls,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        model::class_ &template_instantiation);

    common::model::template_parameter process_template_argument(
        const clang::TemplateArgument &arg);

    std::optional<template_parameter>
    get_template_argument_from_type_parameter_string(
        const clang::Decl *decl, const std::string &return_type_name) const;

    common::visitor::ast_id_mapper &id_mapper();

    clang::SourceManager &source_manager() const;

private:
    // Reference to the output diagram model
    clanguml::class_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::class_diagram &config_;

    common::visitor::ast_id_mapper &id_mapper_;

    clang::SourceManager &source_manager_;
};

}