/**
 * @file src/class_diagram/visitor/template_builder.cc
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
#include "common/clang_utils.h"
#include "translation_unit_visitor.h"
#include <clang/Lex/Lexer.h>

namespace clanguml::class_diagram::visitor {

template_builder::template_builder(
    clanguml::class_diagram::visitor::translation_unit_visitor &visitor)
    : diagram_{visitor.diagram()}
    , config_{visitor.config()}
    , id_mapper_{visitor.id_mapper()}
    , source_manager_{visitor.source_manager()}
    , visitor_{visitor}
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
    auto templated_decl_global_id =
        id_mapper().get_global_id(templated_decl_id).value_or(0);

    if (best_match_id > 0) {
        destination = best_match_full_name;
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, best_match_id});
        template_instantiation.template_specialization_found(true);
    }
    // If we can't find optimal match for parent template specialization,
    // just use whatever clang suggests
    else if (diagram().has_element(templated_decl_global_id)) {
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_global_id});
        template_instantiation.template_specialization_found(true);
    }
    else if (diagram().should_include(
                 namespace_{full_template_specialization_name})) {
        LOG_DBG("Skipping instantiation relationship from {} to {}",
            template_instantiation_ptr->full_name(false), templated_decl_id);
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

    visitor_.set_source_location(*template_decl, *template_instantiation_ptr);

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

    const clang::ClassTemplateDecl *template_decl =
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
        template_instantiation.template_specialization_found(true);
    }
    else if (diagram().has_element(templated_decl_local_id)) {
        // If we can't find optimal match for parent template specialization,
        // just use whatever clang suggests
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_local_id});
        template_instantiation.template_specialization_found(true);
    }
    else if (diagram().should_include(namespace_{qualified_name})) {
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
    auto arg_index{0};

    for (const auto &arg : template_args) {
        // Argument can be a parameter pack in which case it gives multiple
        // arguments
        std::vector<template_parameter> arguments;

        // For now ignore the default template arguments of templates which
        // do not match the inclusion filters, to make the system
        // templates 'nicer' - i.e. skipping the allocators and comparators
        // TODO: Change this to ignore only when the arguments are set to
        //       default values, and add them when they are specifically
        //       overridden
        if (!diagram().should_include(
                namespace_{template_decl->getQualifiedNameAsString()})) {
            const auto *maybe_type_parm_decl =
                clang::dyn_cast<clang::TemplateTypeParmDecl>(
                    template_decl->getTemplateParameters()->getParam(
                        std::min<unsigned>(arg_index,
                            static_cast<unsigned>(
                                template_decl->getTemplateParameters()
                                    ->size()) -
                                1)));
            if (maybe_type_parm_decl != nullptr &&
                maybe_type_parm_decl->hasDefaultArgument()) {
                continue;
            }
        }

        //
        // Handle the template parameter/argument based on its kind
        //
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
                argument, argument.to_string(using_namespace(), false, true));

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
    LOG_DBG("Processing argument {} in template class: {}", argument_index,
        cls->getQualifiedNameAsString());

    switch (arg.getKind()) {
    case clang::TemplateArgument::Null:
        argument.push_back(process_null_argument(arg));
        break;
    case clang::TemplateArgument::Template:
        argument.push_back(process_template_argument(arg));
        break;
    case clang::TemplateArgument::Type: {
        argument.push_back(process_type_argument(parent, cls, template_decl,
            arg.getAsType(), template_instantiation, argument_index));
        break;
    }
    case clang::TemplateArgument::Declaration:
        break;
    case clang::TemplateArgument::NullPtr:
        argument.push_back(process_nullptr_argument(arg));
        break;
    case clang::TemplateArgument::Integral:
        argument.push_back(process_integral_argument(arg));
        break;
    case clang::TemplateArgument::TemplateExpansion:
        argument.push_back(process_template_expansion(arg));
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
    auto arg_name = common::to_string(arg);

    LOG_DBG("Processing template argument: {}", arg_name);

    return template_parameter::make_template_type(arg_name);
}

template_parameter template_builder::process_template_expansion(
    const clang::TemplateArgument &arg)
{
    auto arg_name = common::to_string(arg);

    LOG_DBG("Processing template expansion argument: {}", arg_name);

    util::if_not_null(
        arg.getAsTemplate().getAsTemplateDecl(), [&arg_name](const auto *decl) {
            arg_name = decl->getQualifiedNameAsString();
        });

    auto param = template_parameter::make_template_type(arg_name);
    param.is_variadic(true);

    return param;
}

clang::QualType template_builder::consume_context(
    clang::QualType type, template_parameter &tp) const
{
    auto [unqualified_type, context] = common::consume_type_context(type);

    tp.deduced_context(std::move(context));

    return unqualified_type;
}

template_parameter template_builder::process_type_argument(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType type, class_ &template_instantiation, size_t argument_index)
{
    std::optional<template_parameter> argument;

    if (type->getAs<clang::ElaboratedType>() != nullptr) {
        type = type->getAs<clang::ElaboratedType>()->getNamedType(); // NOLINT
    }

    auto type_name = common::to_string(type, &cls->getASTContext());

    LOG_DBG("Processing template {} type argument {}: {}, {}, {}",
        template_decl->getQualifiedNameAsString(), argument_index, type_name,
        type->getTypeClassName(),
        common::to_string(type, cls->getASTContext()));

    argument = try_as_function_prototype(parent, cls, template_decl, type,
        template_instantiation, argument_index);
    if (argument)
        return *argument;

    argument = try_as_member_pointer(parent, cls, template_decl, type,
        template_instantiation, argument_index);
    if (argument)
        return *argument;

    argument = try_as_array(parent, cls, template_decl, type,
        template_instantiation, argument_index);
    if (argument)
        return *argument;

    argument = try_as_template_parm_type(cls, template_decl, type);
    if (argument)
        return *argument;

    argument = try_as_template_specialization_type(parent, cls, template_decl,
        type, template_instantiation, argument_index);
    if (argument)
        return *argument;

    argument = try_as_decl_type(parent, cls, template_decl, type,
        template_instantiation, argument_index);
    if (argument)
        return *argument;

    argument = try_as_typedef_type(parent, cls, template_decl, type,
        template_instantiation, argument_index);
    if (argument)
        return *argument;

    argument = try_as_lambda(cls, template_decl, type);
    if (argument)
        return *argument;

    argument = try_as_record_type(parent, cls, template_decl, type,
        template_instantiation, argument_index);
    if (argument)
        return *argument;

    argument = try_as_enum_type(
        parent, cls, template_decl, type, template_instantiation);
    if (argument)
        return *argument;

    argument = try_as_builtin_type(parent, type, template_decl);
    if (argument)
        return *argument;

    // fallback
    return template_parameter::make_argument(type_name);
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
    if (config_.generate_template_argument_dependencies() && element_opt) {
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

namespace detail {

std::string map_type_parameter_to_template_parameter(
    const clang::ClassTemplateSpecializationDecl *decl, const std::string &tp)
{
    const auto [depth0, index0, qualifier0] =
        common::extract_template_parameter_index(tp);

    for (auto i = 0U; i < decl->getDescribedTemplateParams()->size(); i++) {
        const auto *param = decl->getDescribedTemplateParams()->getParam(i);

        if (i == index0) {
            return param->getNameAsString();
        }
    }

    return tp;
}

std::string map_type_parameter_to_template_parameter(
    const clang::TypeAliasTemplateDecl *decl, const std::string &tp)
{
    const auto [depth0, index0, qualifier0] =
        common::extract_template_parameter_index(tp);

    for (auto i = 0U; i < decl->getTemplateParameters()->size(); i++) {
        const auto *param = decl->getTemplateParameters()->getParam(i);

        if (i == index0) {
            return param->getNameAsString();
        }
    }

    return tp;
}

} // namespace detail

std::string map_type_parameter_to_template_parameter_name(
    const clang::Decl *decl, const std::string &type_parameter)
{
    if (type_parameter.find("type-parameter-") != 0)
        return type_parameter;

    if (const auto *template_decl =
            llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl);
        template_decl != nullptr) {
        return detail::map_type_parameter_to_template_parameter(
            template_decl, type_parameter);
    }

    if (const auto *alias_decl =
            llvm::dyn_cast<clang::TypeAliasTemplateDecl>(decl);
        alias_decl != nullptr) {
        return detail::map_type_parameter_to_template_parameter(
            alias_decl, type_parameter);
    }

    // Fallback
    return type_parameter;
}

template_parameter template_builder::process_integral_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::Integral);

    std::string result;
    llvm::raw_string_ostream ostream(result);
    arg.dump(ostream);

    return template_parameter::make_argument(result);
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
    const clang::TemplateDecl *base_template_decl,
    const clang::TemplateArgument &arg, size_t argument_index,
    std::vector<template_parameter> & /*argument*/)
{
    assert(arg.getKind() == clang::TemplateArgument::Pack);

    std::vector<template_parameter> res;

    auto pack_argument_index = argument_index;

    for (const auto &a : arg.getPackAsArray()) {
        argument_process_dispatch(parent, cls, template_instantiation,
            base_template_decl, a, pack_argument_index++, res);
    }

    return res;
}

std::optional<template_parameter> template_builder::try_as_member_pointer(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type, class_ &template_instantiation,
    size_t argument_index)
{
    const auto *mp_type =
        common::dereference(type)->getAs<clang::MemberPointerType>();
    if (mp_type == nullptr)
        return {};

    auto argument = template_parameter::make_template_type("");
    type = consume_context(type, argument);

    // Handle a pointer to a data member of a class
    if (mp_type->isMemberDataPointer()) {
        argument.is_member_pointer(false);
        argument.is_data_pointer(true);

        auto pointee_arg = process_type_argument(parent, cls, template_decl,
            mp_type->getPointeeType(), template_instantiation, argument_index);

        argument.add_template_param(std::move(pointee_arg));

        const auto *member_class_type = mp_type->getClass();

        if (member_class_type == nullptr)
            return {};

        auto class_type_arg = process_type_argument(parent, cls, template_decl,
            mp_type->getClass()->getCanonicalTypeUnqualified(),
            template_instantiation, argument_index);

        argument.add_template_param(std::move(class_type_arg));
    }
    // Handle pointer to class method member
    else {
        argument.is_member_pointer(true);
        argument.is_data_pointer(false);

        const auto *function_type =
            mp_type->getPointeeType()->getAs<clang::FunctionProtoType>();

        assert(function_type != nullptr);

        auto return_type_arg = process_type_argument(parent, cls, template_decl,
            function_type->getReturnType(), template_instantiation,
            argument_index);

        // Add return type argument
        argument.add_template_param(std::move(return_type_arg));

        const auto *member_class_type = mp_type->getClass();

        if (member_class_type == nullptr)
            return {};

        auto class_type_arg = process_type_argument(parent, cls, template_decl,
            mp_type->getClass()->getCanonicalTypeUnqualified(),
            template_instantiation, argument_index);

        // Add class type argument
        argument.add_template_param(std::move(class_type_arg));

        // Add argument types
        for (const auto &param_type : function_type->param_types()) {
            argument.add_template_param(
                process_type_argument(parent, cls, template_decl, param_type,
                    template_instantiation, argument_index));
        }
    }

    return argument;
}

std::optional<template_parameter> template_builder::try_as_array(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type, class_ &template_instantiation,
    size_t argument_index)
{
    const auto *array_type = common::dereference(type)->getAsArrayTypeUnsafe();
    if (array_type == nullptr)
        return {};

    auto argument = template_parameter::make_template_type("");

    type = consume_context(type, argument);

    argument.is_array(true);

    // Set function template return type
    auto element_type = process_type_argument(parent, cls, template_decl,
        array_type->getElementType(), template_instantiation, argument_index);

    argument.add_template_param(element_type);

    if (array_type->isDependentSizedArrayType() &&
        array_type->getDependence() ==
            clang::TypeDependence::DependentInstantiation) {
        argument.add_template_param(
            template_parameter::make_template_type(common::to_string(
                ((clang::DependentSizedArrayType *)array_type) // NOLINT
                    ->getSizeExpr())));
    }
    else if (array_type->isConstantArrayType()) {
        argument.add_template_param(template_parameter::make_argument(
            std::to_string(((clang::ConstantArrayType *)array_type) // NOLINT
                               ->getSize()
                               .getLimitedValue())));
    }

    // TODO: Handle variable sized arrays

    return argument;
}

std::optional<template_parameter> template_builder::try_as_function_prototype(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type, class_ &template_instantiation,
    size_t argument_index)
{
    const auto *function_type = type->getAs<clang::FunctionProtoType>();

    if (function_type == nullptr && type->isFunctionPointerType()) {
        function_type =
            type->getPointeeType()->getAs<clang::FunctionProtoType>();
        if (function_type == nullptr)
            return {};
    }

    if (function_type == nullptr)
        return {};

    LOG_DBG("Template argument is a function prototype");

    auto argument = template_parameter::make_template_type("");

    type = consume_context(type, argument);

    argument.is_function_template(true);

    // Set function template return type
    auto return_arg = process_type_argument(parent, cls, template_decl,
        function_type->getReturnType(), template_instantiation, argument_index);

    argument.add_template_param(return_arg);

    // Set function template argument types
    if (function_type->isVariadic() && function_type->param_types().empty()) {
        auto fallback_arg = template_parameter::make_argument({});
        fallback_arg.is_ellipsis(true);
        argument.add_template_param(std::move(fallback_arg));
    }
    else {
        for (const auto &param_type : function_type->param_types()) {
            argument.add_template_param(
                process_type_argument(parent, cls, template_decl, param_type,
                    template_instantiation, argument_index));
        }
    }

    return argument;
}

std::optional<template_parameter> template_builder::try_as_decl_type(
    std::optional<clanguml::class_diagram::model::class_ *> & /*parent*/,
    const clang::NamedDecl * /*cls*/,
    const clang::TemplateDecl * /*template_decl*/, clang::QualType &type,
    class_ & /*template_instantiation*/, size_t /*argument_index*/)
{
    const auto *decl_type =
        common::dereference(type)->getAs<clang::DecltypeType>();
    if (decl_type == nullptr) {
        return {};
    }

    LOG_DBG("Template argument is a decltype()");

    // TODO
    return {};
}

std::optional<template_parameter> template_builder::try_as_typedef_type(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl * /*cls*/,
    const clang::TemplateDecl * /*template_decl*/, clang::QualType &type,
    class_ & /*template_instantiation*/, size_t /*argument_index*/)
{
    const auto *typedef_type =
        common::dereference(type)->getAs<clang::TypedefType>();
    if (typedef_type == nullptr) {
        return {};
    }

    LOG_DBG("Template argument is a typedef/using");

    // If this is a typedef/using alias to a decltype - we're not able
    // to figure out anything out of it probably
    if (typedef_type->getAs<clang::DecltypeType>() != nullptr) {
        // Here we need to figure out the parent context of this alias,
        // it can be a:
        //   - class/struct
        if (typedef_type->getDecl()->isCXXClassMember() && parent) {
            return template_parameter::make_argument(
                fmt::format("{}::{}", parent.value()->full_name(false),
                    typedef_type->getDecl()->getNameAsString()));
        }
        //   - namespace
        return template_parameter::make_argument(
            typedef_type->getDecl()->getQualifiedNameAsString());
    }

    return {};
}

std::optional<template_parameter>
template_builder::try_as_template_specialization_type(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type, class_ &template_instantiation,
    size_t argument_index)
{
    const auto *nested_template_type =
        common::dereference(type)->getAs<clang::TemplateSpecializationType>();
    if (nested_template_type == nullptr) {
        return {};
    }

    LOG_DBG("Template argument is a template specialization type");

    auto argument = template_parameter::make_argument("");
    type = consume_context(type, argument);

    auto nested_type_name = nested_template_type->getTemplateName()
                                .getAsTemplateDecl()
                                ->getQualifiedNameAsString();

    if (clang::dyn_cast<clang::TemplateTemplateParmDecl>(
            nested_template_type->getTemplateName().getAsTemplateDecl()) !=
        nullptr) {
        if (const auto *template_specialization_decl =
                clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(cls);
            template_specialization_decl != nullptr) {
            nested_type_name =
                template_specialization_decl->getDescribedTemplateParams()
                    ->getParam(argument_index)
                    ->getNameAsString();
        }
        else {
            // fallback
            nested_type_name = "template";
        }

        argument.is_template_template_parameter(true);
    }

    argument.set_type(nested_type_name);

    auto nested_template_instantiation = build(cls, *nested_template_type,
        diagram().should_include(
            namespace_{template_decl->getQualifiedNameAsString()})
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
        diagram().should_include(
            namespace_{nested_template_instantiation_full_name})) {
        if (config_.generate_template_argument_dependencies()) {
            if (diagram().should_include(
                    namespace_{template_decl->getQualifiedNameAsString()})) {
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
    }

    if (diagram().should_include(
            namespace_{nested_template_instantiation_full_name})) {
        visitor_.set_source_location(
            *template_decl, *nested_template_instantiation);
        visitor_.add_class(std::move(nested_template_instantiation));
    }

    return argument;
}

std::optional<template_parameter> template_builder::try_as_template_parm_type(
    const clang::NamedDecl *cls, const clang::TemplateDecl * /*template_decl*/,
    clang::QualType &type)
{
    auto is_variadic{false};

    const auto *type_parameter =
        common::dereference(type)->getAs<clang::TemplateTypeParmType>();

    auto type_name = common::to_string(type, &cls->getASTContext());

    if (type_parameter == nullptr) {
        if (const auto *pet =
                common::dereference(type)->getAs<clang::PackExpansionType>();
            pet != nullptr) {
            is_variadic = true;
            type_parameter =
                pet->getPattern()->getAs<clang::TemplateTypeParmType>();
        }
    }

    if (type_parameter == nullptr)
        return {};

    LOG_DBG("Template argument is a template parameter type");

    auto argument = template_parameter::make_template_type("");
    type = consume_context(type, argument);

    auto type_parameter_name = common::to_string(type, cls->getASTContext());
    if (type_parameter_name.empty())
        type_parameter_name = "typename";

    argument.set_name(map_type_parameter_to_template_parameter_name(
        cls, type_parameter_name));

    argument.is_variadic(is_variadic);

    visitor_.ensure_lambda_type_is_relative(type_parameter_name);

    return argument;
}

std::optional<template_parameter> template_builder::try_as_lambda(
    const clang::NamedDecl *cls, const clang::TemplateDecl * /*template_decl*/,
    clang::QualType &type)
{
    auto type_name = common::to_string(type, &cls->getASTContext());

    if (type_name.find("(lambda at ") != 0)
        return {};

    LOG_DBG("Template argument is a lambda");

    auto argument = template_parameter::make_argument("");
    type = consume_context(type, argument);

    visitor_.ensure_lambda_type_is_relative(type_name);
    argument.set_type(type_name);

    return argument;
}

std::optional<template_parameter> template_builder::try_as_record_type(
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    const clang::NamedDecl * /*cls*/, const clang::TemplateDecl *template_decl,
    clang::QualType &type, class_ &template_instantiation,
    size_t /*argument_index*/)
{
    const auto *record_type =
        common::dereference(type)->getAs<clang::RecordType>();
    if (record_type == nullptr)
        return {};

    LOG_DBG("Template argument is a c++ record");

    auto argument = template_parameter::make_argument({});
    type = consume_context(type, argument);

    auto type_name = common::to_string(type, template_decl->getASTContext());

    argument.set_type(type_name);
    const auto type_id = common::to_id(type_name);
    argument.set_id(type_id);

    const auto *class_template_specialization =
        clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(
            record_type->getAsRecordDecl());

    if (class_template_specialization != nullptr) {
        auto tag_argument = build_from_class_template_specialization(
            *class_template_specialization);

        if (tag_argument) {
            argument.set_type(tag_argument->name_and_ns());
            for (const auto &p : tag_argument->template_params())
                argument.add_template_param(p);
            for (auto &r : tag_argument->relationships()) {
                template_instantiation.add_relationship(std::move(r));
            }

            if (config_.generate_template_argument_dependencies() &&
                diagram().should_include(tag_argument->get_namespace())) {
                if (parent.has_value())
                    parent.value()->add_relationship(
                        {relationship_t::kDependency, tag_argument->id()});
                visitor_.set_source_location(*template_decl, *tag_argument);
                visitor_.add_class(std::move(tag_argument));
            }
        }
    }
    else if (const auto *record_type_decl = record_type->getAsRecordDecl();
             record_type_decl != nullptr) {
        if (config_.generate_template_argument_dependencies() &&
            diagram().should_include(namespace_{type_name})) {
            // Add dependency relationship to the parent
            // template
            template_instantiation.add_relationship(
                {relationship_t::kDependency, type_id});
        }
    }

    return argument;
}

std::optional<template_parameter> template_builder::try_as_enum_type(
    std::optional<clanguml::class_diagram::model::class_ *> & /*parent*/,
    const clang::NamedDecl * /*cls*/, const clang::TemplateDecl *template_decl,
    clang::QualType &type, class_ &template_instantiation)
{
    const auto *enum_type = type->getAs<clang::EnumType>();
    if (enum_type == nullptr)
        return {};

    LOG_DBG("Template argument is a an enum");

    auto argument = template_parameter::make_argument({});
    type = consume_context(type, argument);

    auto type_name = common::to_string(type, template_decl->getASTContext());
    argument.set_type(type_name);
    const auto type_id = common::to_id(type_name);
    argument.set_id(type_id);

    if (enum_type->getAsTagDecl() != nullptr &&
        config_.generate_template_argument_dependencies()) {
        template_instantiation.add_relationship(
            {relationship_t::kDependency, type_id});
    }

    return argument;
}

std::optional<template_parameter> template_builder::try_as_builtin_type(
    std::optional<clanguml::class_diagram::model::class_ *> & /*parent*/,
    clang::QualType &type, const clang::TemplateDecl *template_decl)
{
    const auto *builtin_type = type->getAs<clang::BuiltinType>();
    if (builtin_type == nullptr)
        return {};

    LOG_DBG("Template argument is a builtin type");

    auto type_name = common::to_string(type, template_decl->getASTContext());
    auto argument = template_parameter::make_argument(type_name);

    type = consume_context(type, argument);
    argument.set_type(type_name);

    return argument;
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

} // namespace clanguml::class_diagram::visitor
