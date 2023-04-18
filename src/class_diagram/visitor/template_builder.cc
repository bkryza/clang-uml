/**
 * src/class_diagram/visitor/template_builder.cc
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

#include "template_builder.h"
#include "translation_unit_visitor.h"

namespace clanguml::class_diagram::visitor {

template_builder::template_builder(class_diagram::model::diagram &d,
    const config::class_diagram &config,
    common::visitor::ast_id_mapper &id_mapper,
    clang::SourceManager &source_manager)
    : diagram_{d}
    , config_{config}
    , id_mapper_{id_mapper}
    , source_manager_{source_manager}
{
}

class_diagram::model::diagram &template_builder::diagram() { return diagram_; }

const config::class_diagram &template_builder::config() const
{
    return config_;
}

const namespace_ &template_builder::using_namespace() const
{
    return config_.using_namespace();
}

common::visitor::ast_id_mapper &template_builder::id_mapper()
{
    return id_mapper_;
}

clang::SourceManager &template_builder::source_manager() const
{
    return source_manager_;
}

bool template_builder::simplify_system_template(
    template_parameter &ct, const std::string &full_name) const
{
    auto simplified = config().simplify_template_type(full_name);

    if (simplified != full_name) {
        ct.set_type(simplified);
        ct.clear_params();
        return true;
    }

    return false;
}

std::unique_ptr<class_> template_builder::build(const clang::NamedDecl *cls,
    const clang::TemplateSpecializationType &template_type_decl,
    std::optional<clanguml::class_diagram::model::class_ *> parent)
{
    //
    // Here we'll hold the template base class params to replace with the
    // instantiated values
    //
    std::deque<std::tuple</*parameter name*/ std::string, /* position */ int,
        /*is variadic */ bool>>
        template_base_params{};

    const auto *template_type_ptr = &template_type_decl;

    if (template_type_decl.isTypeAlias()) {
        if (const auto *tsp =
                template_type_decl.getAliasedType()
                    ->template getAs<clang::TemplateSpecializationType>();
            tsp != nullptr)
            template_type_ptr = tsp;
    }

    const auto &template_type = *template_type_ptr;

    //
    // Create class_ instance to hold the template instantiation
    //
    auto template_instantiation_ptr =
        std::make_unique<class_>(using_namespace());

    auto &template_instantiation = *template_instantiation_ptr;
    template_instantiation.is_template(true);

    std::string full_template_specialization_name = common::to_string(
        template_type.desugar(),
        template_type.getTemplateName().getAsTemplateDecl()->getASTContext());

    auto *template_decl{template_type.getTemplateName().getAsTemplateDecl()};

    auto template_decl_qualified_name =
        template_decl->getQualifiedNameAsString();

    auto *class_template_decl{
        clang::dyn_cast<clang::ClassTemplateDecl>(template_decl)};

    if ((class_template_decl != nullptr) &&
        (class_template_decl->getTemplatedDecl() != nullptr) &&
        (class_template_decl->getTemplatedDecl()->getParent() != nullptr) &&
        class_template_decl->getTemplatedDecl()->getParent()->isRecord()) {

        namespace_ ns{
            common::get_tag_namespace(*class_template_decl->getTemplatedDecl()
                                           ->getParent()
                                           ->getOuterLexicalRecordContext())};

        std::string ns_str = ns.to_string();
        std::string name = template_decl->getQualifiedNameAsString();
        if (!ns_str.empty()) {
            name = name.substr(ns_str.size() + 2);
        }

        util::replace_all(name, "::", "##");
        template_instantiation.set_name(name);

        template_instantiation.set_namespace(ns);
    }
    else {
        namespace_ ns{template_decl_qualified_name};
        ns.pop_back();
        template_instantiation.set_name(template_decl->getNameAsString());
        template_instantiation.set_namespace(ns);
    }

    // TODO: Refactor handling of base parameters to a separate method

    // We need this to match any possible base classes coming from template
    // arguments
    std::vector<
        std::pair</* parameter name */ std::string, /* is variadic */ bool>>
        template_parameter_names{};

    for (const auto *parameter : *template_decl->getTemplateParameters()) {
        if (parameter->isTemplateParameter() &&
            (parameter->isTemplateParameterPack() ||
                parameter->isParameterPack())) {
            template_parameter_names.emplace_back(
                parameter->getNameAsString(), true);
        }
        else
            template_parameter_names.emplace_back(
                parameter->getNameAsString(), false);
    }

    // Check if the primary template has any base classes
    int base_index = 0;

    const auto *templated_class_decl =
        clang::dyn_cast_or_null<clang::CXXRecordDecl>(
            template_decl->getTemplatedDecl());

    if ((templated_class_decl != nullptr) &&
        templated_class_decl->hasDefinition())
        for (const auto &base : templated_class_decl->bases()) {
            const auto base_class_name = common::to_string(
                base.getType(), templated_class_decl->getASTContext(), false);

            LOG_DBG("Found template instantiation base: {}, {}",
                base_class_name, base_index);

            // Check if any of the primary template arguments has a
            // parameter equal to this type
            auto it = std::find_if(template_parameter_names.begin(),
                template_parameter_names.end(),
                [&base_class_name](
                    const auto &p) { return p.first == base_class_name; });

            if (it != template_parameter_names.end()) {
                const auto &parameter_name = it->first;
                const bool is_variadic = it->second;
                // Found base class which is a template parameter
                LOG_DBG("Found base class which is a template parameter "
                        "{}, {}, {}",
                    parameter_name, is_variadic,
                    std::distance(template_parameter_names.begin(), it));

                template_base_params.emplace_back(parameter_name,
                    std::distance(template_parameter_names.begin(), it),
                    is_variadic);
            }
            else {
                // This is a regular base class - it is handled by
                // process_template
            }
            base_index++;
        }

    process_template_arguments(parent, cls, template_base_params,
        template_type.template_arguments(), template_instantiation,
        template_decl);

    // First try to find the best match for this template in partially
    // specialized templates
    std::string destination{};
    std::string best_match_full_name{};
    auto full_template_name = template_instantiation.full_name(false);
    int best_match{};
    common::model::diagram_element::id_t best_match_id{0};

    for (const auto templ : diagram().classes()) {
        if (templ.get() == template_instantiation)
            continue;

        auto c_full_name = templ.get().full_name(false);
        auto match =
            template_instantiation.calculate_template_specialization_match(
                templ.get());

        if (match > best_match) {
            best_match = match;
            best_match_full_name = c_full_name;
            best_match_id = templ.get().id();
        }
    }

    auto templated_decl_id =
        template_type.getTemplateName().getAsTemplateDecl()->getID();
    auto templated_decl_local_id =
        id_mapper().get_global_id(templated_decl_id).value_or(0);

    if (best_match_id > 0) {
        destination = best_match_full_name;
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, best_match_id});
    }
    // If we can't find optimal match for parent template specialization,
    // just use whatever clang suggests
    else if (diagram().has_element(templated_decl_local_id)) {
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_local_id});
    }
    else {
        LOG_DBG("== Cannot determine global id for specialization template {} "
                "- delaying until the translation unit is complete ",
            templated_decl_id);

        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_id});
    }

    template_instantiation.set_id(
        common::to_id(template_instantiation_ptr->full_name(false)));

    return template_instantiation_ptr;
}

std::unique_ptr<class_>
template_builder::build_from_class_template_specialization(
    const clang::ClassTemplateSpecializationDecl &template_specialization,
    std::optional<clanguml::class_diagram::model::class_ *> parent)
{
    auto template_instantiation_ptr =
        std::make_unique<class_>(config_.using_namespace());

    //
    // Here we'll hold the template base params to replace with the
    // instantiated values
    //
    std::deque<std::tuple</*parameter name*/ std::string, /* position */ int,
        /*is variadic */ bool>>
        template_base_params{};

    auto &template_instantiation = *template_instantiation_ptr;
    template_instantiation.is_struct(template_specialization.isStruct());

    const auto *template_decl =
        template_specialization.getSpecializedTemplate();

    auto qualified_name = template_decl->getQualifiedNameAsString();

    namespace_ ns{qualified_name};
    ns.pop_back();
    template_instantiation.set_name(template_decl->getNameAsString());
    template_instantiation.set_namespace(ns);

    process_template_arguments(parent, &template_specialization,
        template_base_params,
        template_specialization.getTemplateArgs().asArray(),
        template_instantiation, template_decl);

    // Update the id after the template parameters are processed
    template_instantiation.set_id(
        common::to_id(template_instantiation.full_name(false)));

    // First try to find the best match for this template in partially
    // specialized templates
    std::string destination{};
    std::string best_match_full_name{};
    auto full_template_name = template_instantiation.full_name(false);
    int best_match{};
    common::model::diagram_element::id_t best_match_id{0};

    for (const auto templ : diagram().classes()) {
        if (templ.get() == template_instantiation)
            continue;

        auto c_full_name = templ.get().full_name(false);
        auto match =
            template_instantiation.calculate_template_specialization_match(
                templ.get());

        if (match > best_match) {
            best_match = match;
            best_match_full_name = c_full_name;
            best_match_id = templ.get().id();
        }
    }

    auto templated_decl_id = template_specialization.getID();
    auto templated_decl_local_id =
        id_mapper().get_global_id(templated_decl_id).value_or(0);

    if (best_match_id > 0) {
        destination = best_match_full_name;
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, best_match_id});
    }
    // If we can't find optimal match for parent template specialization,
    // just use whatever clang suggests
    else if (diagram().has_element(templated_decl_local_id)) {
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_local_id});
    }
    else if (diagram().should_include(qualified_name)) {
        LOG_DBG("Skipping instantiation relationship from {} to {}",
            template_instantiation_ptr->full_name(false), templated_decl_id);
    }

    return template_instantiation_ptr;
}

void template_builder::process_template_arguments(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls,
    std::deque<std::tuple<std::string, int, bool>> &template_base_params,
    const clang::ArrayRef<clang::TemplateArgument> &template_args,
    class_ &template_instantiation, const clang::TemplateDecl *template_decl)
{
    auto arg_index = 0;
    for (const auto &arg : template_args) {
        // Argument can be a parameter pack in which case it gives multiple
        // arguments
        std::vector<template_parameter> arguments;

        argument_process_dispatch(parent, cls, template_instantiation,
            template_decl, arg, arg_index, arguments);

        if (arguments.empty()) {
            arg_index++;
            continue;
        }

        // We can figure this only when we encounter variadic param in
        // the list of template params, from then this variable is true
        // and we can process following template parameters as belonging
        // to the variadic tuple
        [[maybe_unused]] auto variadic_params{false};

        // In case any of the template arguments are base classes, add
        // them as parents of the current template instantiation class
        if (!template_base_params.empty()) {
            variadic_params =
                add_base_classes(template_instantiation, template_base_params,
                    arg_index, variadic_params, arguments.front());
        }

        for (auto &argument : arguments) {
            simplify_system_template(
                argument, argument.to_string(using_namespace(), false));

            LOG_DBG("Adding template argument {} to template "
                    "specialization/instantiation {}",
                argument.to_string(using_namespace(), false),
                template_instantiation.name());

            template_instantiation.add_template(std::move(argument));
        }

        arg_index++;
    }
}

void template_builder::argument_process_dispatch(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, class_ &template_instantiation,
    const clang::TemplateDecl *template_decl,
    const clang::TemplateArgument &arg, size_t argument_index,
    std::vector<template_parameter> &argument)
{
    switch (arg.getKind()) {
    case clang::TemplateArgument::Null:
        argument.push_back(process_null_argument(arg));
        break;
    case clang::TemplateArgument::Type:
        argument.push_back(process_type_argument(parent, cls, template_decl,
            arg, template_instantiation, argument_index));
        break;
    case clang::TemplateArgument::Declaration:
        break;
    case clang::TemplateArgument::NullPtr:
        argument.push_back(process_nullptr_argument(arg));
        break;
    case clang::TemplateArgument::Integral:
        argument.push_back(process_integral_argument(arg));
        break;
    case clang::TemplateArgument::Template:
        argument.push_back(process_template_argument(arg));
        break;
    case clang::TemplateArgument::TemplateExpansion:
        break;
    case clang::TemplateArgument::Expression:
        argument.push_back(process_expression_argument(arg));
        break;
    case clang::TemplateArgument::Pack:
        for (auto &a :
            process_pack_argument(parent, cls, template_instantiation,
                template_decl, arg, argument_index, argument)) {
            argument.push_back(a);
        }
        break;
    }
}

template_parameter template_builder::process_template_argument(
    const clang::TemplateArgument &arg)
{
    LOG_DBG("Processing template argument: {}", common::to_string(arg));

    auto arg_name =
        arg.getAsTemplate().getAsTemplateDecl()->getQualifiedNameAsString();
    return template_parameter::make_template_type(arg_name);
}

template_parameter template_builder::process_type_argument(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    const clang::TemplateArgument &arg, class_ &template_instantiation,
    size_t argument_index)
{
    assert(arg.getKind() == clang::TemplateArgument::Type);

    auto type_name = common::to_string(arg, &cls->getASTContext());

    LOG_DBG("Processing template {} type argument: {}",
        template_decl->getQualifiedNameAsString(), type_name);

    auto argument = template_parameter::make_argument({});

    if (const auto *function_type =
            arg.getAsType()->getAs<clang::FunctionProtoType>();
        function_type != nullptr) {

        argument.set_function_template(true);

        // Set function template return type
        const auto return_type_name =
            function_type->getReturnType().getAsString();

        // Try to match the return type to template parameter in case
        // the type name is in the form 'type-parameter-X-Y'
        auto maybe_return_arg =
            get_template_argument_from_type_parameter_string(
                cls, return_type_name);

        if (maybe_return_arg)
            argument.add_template_param(*maybe_return_arg);
        else {
            argument.add_template_param(
                template_parameter::make_argument(return_type_name));
        }

        // Set function template argument types
        for (const auto &param_type : function_type->param_types()) {
            auto maybe_arg = get_template_argument_from_type_parameter_string(
                cls, param_type.getAsString());

            if (maybe_arg) {
                argument.add_template_param(*maybe_arg);
                continue;
            }

            if (param_type->isBuiltinType()) {
                argument.add_template_param(template_parameter::make_argument(
                    param_type.getAsString()));
                continue;
            }

            const auto *param_record_type =
                param_type->getAs<clang::RecordType>();
            if (param_record_type == nullptr)
                continue;

            auto *classTemplateSpecialization =
                llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(
                    param_type->getAsRecordDecl());

            if (classTemplateSpecialization != nullptr) {
                // Read arg info as needed.
                auto nested_template_instantiation =
                    build_from_class_template_specialization(
                        *classTemplateSpecialization, // *param_record_type,
                        diagram().should_include(
                            template_decl->getQualifiedNameAsString())
                            ? std::make_optional(&template_instantiation)
                            : parent);

                const auto nested_template_name =
                    classTemplateSpecialization->getQualifiedNameAsString();

                if (nested_template_instantiation) {
                    if (parent.has_value())
                        parent.value()->add_relationship(
                            {relationship_t::kDependency,
                                nested_template_instantiation->id()});
                }

                auto nested_template_instantiation_full_name =
                    nested_template_instantiation->full_name(false);
                if (diagram().should_include(
                        nested_template_instantiation_full_name)) {
                    diagram().add_class(
                        std::move(nested_template_instantiation));
                }
            }
        }
    }
    else if (const auto *nested_template_type =
                 arg.getAsType()->getAs<clang::TemplateSpecializationType>();
             nested_template_type != nullptr) {

        const auto nested_type_name = nested_template_type->getTemplateName()
                                          .getAsTemplateDecl()
                                          ->getQualifiedNameAsString();

        argument.set_type(nested_type_name);

        auto nested_template_instantiation = build(cls, *nested_template_type,
            diagram().should_include(template_decl->getQualifiedNameAsString())
                ? std::make_optional(&template_instantiation)
                : parent);

        argument.set_id(nested_template_instantiation->id());

        for (const auto &t : nested_template_instantiation->template_params())
            argument.add_template_param(t);

        // Check if this template should be simplified (e.g. system
        // template aliases such as 'std:basic_string<char>' should
        // be simply 'std::string')
        simplify_system_template(
            argument, argument.to_string(using_namespace(), false));

        const auto nested_template_instantiation_full_name =
            nested_template_instantiation->full_name(false);

        if (nested_template_instantiation &&
            diagram().should_include(nested_template_instantiation_full_name)) {
            if (diagram().should_include(
                    template_decl->getQualifiedNameAsString())) {
                template_instantiation.add_relationship(
                    {relationship_t::kDependency,
                        nested_template_instantiation->id()});
            }
            else {
                if (parent.has_value())
                    parent.value()->add_relationship(
                        {relationship_t::kDependency,
                            nested_template_instantiation->id()});
            }
        }

        if (diagram().should_include(nested_template_instantiation_full_name)) {
            diagram().add_class(std::move(nested_template_instantiation));
        }
    }
    else if (arg.getAsType()->getAs<clang::TemplateTypeParmType>() != nullptr) {
        argument = template_parameter::make_template_type({});

        auto parameter_name =
            common::to_string(arg.getAsType(), cls->getASTContext());

        ensure_lambda_type_is_relative(parameter_name);

        // clang does not provide declared template parameter/argument
        // names in template specializations - so we have to extract
        // them from raw source code...
        if (parameter_name.find("type-parameter-") == 0) {
            auto maybe_arg = get_template_argument_from_type_parameter_string(
                cls, parameter_name);

            if (maybe_arg) {
                return *maybe_arg;
            }
        }

        argument.set_name(parameter_name);
    }
    // Case for unexposed template
    else if ((type_name.find('<') != std::string::npos) ||
        (type_name.find("type-parameter-") == 0)) {

        ensure_lambda_type_is_relative(type_name);
        if (type_name.find('<') != std::string::npos) {
            argument = template_parameter::make_argument({});

            // Sometimes template instantiation is reported as
            // RecordType in the AST and getAs to
            // TemplateSpecializationType returns null pointer so we
            // have to at least make sure it's properly formatted
            // (e.g. std:integral_constant, or any template
            // specialization which contains it - see t00038)
            process_unexposed_template_specialization_parameters(
                type_name.substr(type_name.find('<') + 1,
                    type_name.size() - (type_name.find('<') + 2)),
                argument, template_instantiation);

            auto unexposed_type_name = type_name.substr(0, type_name.find('<'));
            ensure_lambda_type_is_relative(unexposed_type_name);

            argument.set_type(unexposed_type_name);
        }
        else if (type_name.find("type-parameter-") == 0) {
            auto maybe_arg = get_template_argument_from_type_parameter_string(
                cls, type_name);

            if (maybe_arg) {
                return *maybe_arg;
            }

            // Otherwise just set the name for the template argument to
            // whatever clang says
            return template_parameter::make_template_type(type_name);
        }
    }

    else if (const auto maybe_arg =
                 get_template_argument_from_type_parameter_string(
                     cls, arg.getAsType().getAsString());
             maybe_arg) {
        // The type is only in the form 'type-parameter-X-Y' so we have
        // to match it to a template parameter name in the 'cls' template
        argument = *maybe_arg;
    }
    else if (type_name.find("(lambda at ") == 0) {
        // This is just a lambda reference
        ensure_lambda_type_is_relative(type_name);
        argument.set_type(type_name);
    }
    else {
        // This is just a regular record type
        process_tag_argument(
            template_instantiation, template_decl, arg, argument);
    }

    return argument;
}

void template_builder::process_unexposed_template_specialization_parameters(
    const std::string &type_name, template_parameter &tp, class_ &c)
{
    auto template_params = common::parse_unexposed_template_params(
        type_name, [](const std::string &t) { return t; });

    found_relationships_t relationships;
    for (auto &param : template_params) {
        find_relationships_in_unexposed_template_params(param, relationships);
        tp.add_template_param(param);
    }

    for (auto &r : relationships) {
        c.add_relationship({std::get<1>(r), std::get<0>(r)});
    }
}

bool template_builder::find_relationships_in_unexposed_template_params(
    const template_parameter &ct, found_relationships_t &relationships)
{
    const auto &type = ct.type();

    if (!type)
        return false;

    bool found{false};
    LOG_DBG("Finding relationships in user defined type: {}",
        ct.to_string(config().using_namespace(), false));

    auto type_with_namespace =
        std::make_optional<common::model::namespace_>(type.value());

    if (!type_with_namespace.has_value()) {
        // Couldn't find declaration of this type
        type_with_namespace = common::model::namespace_{type.value()};
    }

    auto element_opt = diagram().get(type_with_namespace.value().to_string());
    if (element_opt) {
        relationships.emplace_back(
            element_opt.value().id(), relationship_t::kDependency);
        found = true;
    }

    for (const auto &nested_template_params : ct.template_params()) {
        found = find_relationships_in_unexposed_template_params(
                    nested_template_params, relationships) ||
            found;
    }

    return found;
}

std::optional<template_parameter>
template_builder::get_template_argument_from_type_parameter_string(
    const clang::Decl *decl, const std::string &type_name) const
{
    if (const auto *template_decl =
            llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl);
        template_decl != nullptr && type_name.find("type-parameter-") == 0) {

        if (type_name.rfind("type-parameter-") > 0 &&
            type_name.find("::*") != std::string::npos) {
            if (type_name.find("::*)") == std::string::npos) {
                // This is a method invocation template, e.g.
                // template <typename M, typename C>
                // struct invocable<type-parameter-0-0 type-parameter-0-1::*>
                // {};
                const auto [depth0, index0, qualifier0] =
                    common::extract_template_parameter_index(
                        type_name.substr(0, type_name.find(' ')));

                const auto [depth1, index1, qualifier1] =
                    common::extract_template_parameter_index(type_name.substr(
                        type_name.find(' ') + 1, type_name.find(':')));

                auto template_param =
                    template_parameter::make_template_type({});
                template_param.set_method_template(true);

                std::string param_name = type_name;

                for (auto i = 0U;
                     i < template_decl->getDescribedTemplateParams()->size();
                     i++) {
                    const auto *param =
                        template_decl->getDescribedTemplateParams()->getParam(
                            i);

                    if (i == index0) {
                        param_name = param->getNameAsString();

                        template_param.add_template_param(
                            template_parameter::make_template_type(param_name));
                    }

                    if (i == index1) {
                        param_name = param->getNameAsString();

                        template_param.add_template_param(
                            template_parameter::make_template_type(param_name));
                    }
                }

                template_param.set_method_qualifier(
                    type_name.substr(type_name.find("::*") + 3));

                return template_param;
            }
            else {
                // Here we're dealing with method type with args, e.g.:
                // type-parameter-0-0
                // (type-parameter-0-1::*)(type-parameter-0-2)
                const auto [depth0, index0, qualifier0] =
                    common::extract_template_parameter_index(
                        type_name.substr(0, type_name.find(' ')));

                const auto [depth1, index1, qualifier1] =
                    common::extract_template_parameter_index(type_name.substr(
                        type_name.find(" (") + 2, type_name.find(':')));

                auto template_param =
                    template_parameter::make_template_type({});
                template_param.set_method_template(true);

                // TODO: Handle args
                for (auto i = 0U;
                     i < template_decl->getDescribedTemplateParams()->size();
                     i++) {
                    const auto *param =
                        template_decl->getDescribedTemplateParams()->getParam(
                            i);

                    if (i == index0) {
                        template_param.add_template_param(
                            template_parameter::make_template_type(
                                param->getNameAsString()));
                    }

                    if (i == index1) {
                        template_param.add_template_param(
                            template_parameter::make_template_type(
                                param->getNameAsString()));
                    }
                }

                return template_param;
            }
        }
        else {
            const auto [depth, index, qualifier] =
                common::extract_template_parameter_index(type_name);

            std::string param_name = type_name;

            clang::ClassTemplateSpecializationDecl *template_decl_at_depth =
                const_cast<clang::ClassTemplateSpecializationDecl *>(
                    template_decl);

            for (auto i = 0U; i <
                 template_decl_at_depth->getDescribedTemplateParams()->size();
                 i++) {
                const auto *param =
                    template_decl_at_depth->getDescribedTemplateParams()
                        ->getParam(i);

                if (i == index) {
                    param_name = param->getNameAsString();

                    auto template_param =
                        template_parameter::make_template_type(param_name);

                    template_param.is_variadic(param->isParameterPack());

                    return template_param;
                }
            }
        }
    }
    else if (const auto *alias_decl =
                 llvm::dyn_cast<clang::TypeAliasTemplateDecl>(decl);
             alias_decl != nullptr && type_name.find("type-parameter-") == 0) {
        const auto [depth, index, qualifier] =
            common::extract_template_parameter_index(type_name);

        std::string param_name = type_name;

        const auto *template_decl_at_depth = alias_decl;

        template_decl_at_depth->dump();

        for (auto i = 0U;
             i < template_decl_at_depth->getTemplateParameters()->size(); i++) {
            const auto *param =
                template_decl_at_depth->getTemplateParameters()->getParam(i);

            if (i == index) {
                param_name = param->getNameAsString();

                auto template_param =
                    template_parameter::make_template_type(param_name);

                template_param.is_variadic(param->isParameterPack());

                return template_param;
            }
        }
    }

    return {};
}

template_parameter template_builder::process_integral_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::Integral);

    return template_parameter::make_argument(
        std::to_string(arg.getAsIntegral().getExtValue()));
}

template_parameter template_builder::process_null_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::Null);

    return template_parameter::make_argument("");
}

template_parameter template_builder::process_nullptr_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::NullPtr);

    LOG_DBG("Processing nullptr argument: {}", common::to_string(arg));

    return template_parameter::make_argument("nullptr");
}

template_parameter template_builder::process_expression_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::Expression);
    return template_parameter::make_argument(common::get_source_text(
        arg.getAsExpr()->getSourceRange(), source_manager()));
}

std::vector<template_parameter> template_builder::process_pack_argument(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, class_ &template_instantiation,
    const clang::TemplateDecl *template_decl,
    const clang::TemplateArgument &arg, size_t argument_index,
    std::vector<template_parameter> &argument)
{
    assert(arg.getKind() == clang::TemplateArgument::Pack);

    std::vector<template_parameter> res;

    int pack_argument_index = argument_index;

    for (const auto &a : arg.getPackAsArray()) {
        argument_process_dispatch(parent, cls, template_instantiation,
            template_decl, a, pack_argument_index++, res);
    }

    return res;
}

void template_builder::process_tag_argument(class_ &template_instantiation,
    const clang::TemplateDecl *template_decl,
    const clang::TemplateArgument &arg, template_parameter &argument)
{
    assert(arg.getKind() == clang::TemplateArgument::Type);

    argument.is_template_parameter(false);

    argument.set_type(
        common::to_string(arg.getAsType(), template_decl->getASTContext()));

    if (const auto *tsp =
            arg.getAsType()->getAs<clang::TemplateSpecializationType>();
        tsp != nullptr) {
        if (const auto *record_type_decl = tsp->getAsRecordDecl();
            record_type_decl != nullptr) {

            argument.set_id(common::to_id(arg));
            if (diagram().should_include(
                    template_decl->getQualifiedNameAsString())) {
                // Add dependency relationship to the parent
                // template
                template_instantiation.add_relationship(
                    {relationship_t::kDependency, common::to_id(arg)});
            }
        }
    }
    else if (const auto *record_type =
                 arg.getAsType()->getAs<clang::RecordType>();
             record_type != nullptr) {
        if (const auto *record_type_decl = record_type->getAsRecordDecl();
            record_type_decl != nullptr) {

            argument.set_id(common::to_id(arg));
#if LLVM_VERSION_MAJOR >= 16
            argument.set_type(record_type_decl->getQualifiedNameAsString());
#endif
            if (diagram().should_include(
                    template_decl->getQualifiedNameAsString())) {
                // Add dependency relationship to the parent
                // template
                template_instantiation.add_relationship(
                    {relationship_t::kDependency, common::to_id(arg)});
            }
        }
    }
    else if (const auto *enum_type = arg.getAsType()->getAs<clang::EnumType>();
             enum_type != nullptr) {
        if (enum_type->getAsTagDecl() != nullptr) {
            template_instantiation.add_relationship(
                {relationship_t::kDependency, common::to_id(arg)});
        }
    }
}

bool template_builder::add_base_classes(class_ &tinst,
    std::deque<std::tuple<std::string, int, bool>> &template_base_params,
    int arg_index, bool variadic_params, const template_parameter &ct)
{
    bool add_template_argument_as_base_class = false;

    auto [arg_name, index, is_variadic] = template_base_params.front();
    if (variadic_params)
        add_template_argument_as_base_class = true;
    else {
        variadic_params = is_variadic;
        if ((arg_index == index) || (is_variadic && arg_index >= index)) {
            add_template_argument_as_base_class = true;
            if (!is_variadic) {
                // Don't remove the remaining variadic parameter
                template_base_params.pop_front();
            }
        }
    }

    const auto maybe_id = ct.id();
    if (add_template_argument_as_base_class && maybe_id) {
        LOG_DBG("Adding template argument as base class '{}'",
            ct.to_string({}, false));

        model::class_parent cp;
        cp.set_access(common::model::access_t::kPublic);
        cp.set_name(ct.to_string({}, false));
        cp.set_id(maybe_id.value());

        tinst.add_parent(std::move(cp));
    }

    return variadic_params;
}

void template_builder::ensure_lambda_type_is_relative(
    std::string &parameter_type) const
{
#ifdef _MSC_VER
    auto root_name = fmt::format(
        "{}\\", std::filesystem::current_path().root_name().string());
    if (root_name.back() == '\\') {
        root_name.pop_back();
        root_name.push_back('/');
    }
#else
    auto root_name = std::string{"/"};
#endif

    std::string lambda_prefix{fmt::format("(lambda at {}", root_name)};

    while (parameter_type.find(lambda_prefix) != std::string::npos) {
        auto lambda_begin = parameter_type.find(lambda_prefix);

        auto absolute_lambda_path_end =
            parameter_type.find(':', lambda_begin + lambda_prefix.size());
        auto absolute_lambda_path =
            parameter_type.substr(lambda_begin + lambda_prefix.size() - 1,
                absolute_lambda_path_end -
                    (lambda_begin + lambda_prefix.size() - 1));

        auto relative_lambda_path = util::path_to_url(std::filesystem::relative(
            absolute_lambda_path, config().relative_to())
                                                          .string());

        parameter_type = fmt::format("{}(lambda at {}{}",
            parameter_type.substr(0, lambda_begin), relative_lambda_path,
            parameter_type.substr(absolute_lambda_path_end));
    }
}

} // namespace clanguml::class_diagram::visitor
