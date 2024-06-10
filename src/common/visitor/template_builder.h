/**
 * @file src/class_diagram/visitor/template_builder.h
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

#include "class_diagram/model/diagram.h"
#include "common/model/diagram.h"
#include "common/model/template_element.h"
#include "common/visitor/ast_id_mapper.h"
#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"

namespace clanguml::common::visitor {

using common::model::namespace_;
using common::model::relationship_t;
using common::model::template_parameter;

using found_relationships_t =
    std::vector<std::pair<eid_t, common::model::relationship_t>>;

namespace detail {

std::string map_type_parameter_to_template_parameter(
    const clang::ClassTemplateSpecializationDecl *decl, const std::string &tp);

std::string map_type_parameter_to_template_parameter(
    const clang::TypeAliasTemplateDecl *decl, const std::string &tp);

} // namespace detail

std::string map_type_parameter_to_template_parameter_name(
    const clang::Decl *decl, const std::string &type_parameter);

/**
 * @brief Class responsible for building all kinds of templates from Clang AST.
 */
template <typename VisitorT> class template_builder {
public:
    /**
     * @brief Constructor.
     *
     * @param visitor Reference to class diagram translation_unit_visitor
     */
    template_builder(clanguml::common::model::diagram &diagram_,
        const clanguml::config::diagram &config_, VisitorT &visitor);

    /**
     * @brief Get reference to the current diagram model
     *
     * @return Reference to the current diagram model
     */
    common::model::diagram &diagram();

    /**
     * @brief Get reference to the current diagram configuration
     *
     * @return Reference to the current diagram configuration
     */
    const config::diagram &config() const;

    /**
     * @brief Get diagram relative namespace
     *
     * @return Diagram relative namespace
     */
    const namespace_ &using_namespace() const;

    /**
     * @brief Simplify system templates
     *
     * This method tries to simplify all fully qualified template names
     * in the `full_name` using substitutions from the configuration file
     * ().
     *
     * Typical example is replace every `std::basic_string<char>` with
     * `std::string`.
     *
     * @param ct Template parameter
     * @param full_name Full template name
     * @return
     */
    bool simplify_system_template(
        template_parameter &ct, const std::string &full_name) const;

    void build_from_template_declaration(
        clanguml::common::model::template_trait &template_model,
        const clang::TemplateDecl &template_declaration,
        common::optional_ref<common::model::element> templated_element = {});

    /**
     * @brief Basic template class build method
     *
     * @param cls Clang template declaration
     * @param template_type_decl Template specialization type
     * @param parent Optional class in which this template is contained
     * @return Created template class model
     */
    void build_from_template_specialization_type(
        clanguml::common::model::template_element &template_instantiation,
        const clang::NamedDecl *cls,
        const clang::TemplateSpecializationType &template_type_decl,
        std::optional<clanguml::common::model::template_element *> parent = {});

    void build(
        clanguml::common::model::template_element &template_instantiation,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::ArrayRef<clang::TemplateArgument> template_arguments,
        std::string full_template_specialization_name,
        std::optional<clanguml::common::model::template_element *> parent = {});

    /**
     * @brief Build template class from class template specialization decl
     *
     * @param template_specialization Class template specialization declaration
     * @param parent Optional class in which this template is contained
     * @return Created template class model
     */
    void build_from_class_template_specialization(
        clanguml::common::model::template_element &template_instantiation,
        const clang::ClassTemplateSpecializationDecl &template_specialization,
        std::optional<clanguml::common::model::template_element *> parent = {});

    /**
     * @brief Add base classes to the template class, if any.
     *
     * This method adds base classes to a template declaration or
     * specialization, including inferring whether variadic template
     * parameter bases.
     *
     * @param tinst Class template model
     * @param template_base_params List of base class template parameters
     * @param arg_index Index of the template argument used for base class
     * @param variadic_params Whether the parameter is variadic
     * @param ct Template parameter model
     * @return True, if any base classes were added
     */
    bool add_base_classes(clanguml::common::model::template_element &tinst,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        int arg_index, bool variadic_params,
        const clanguml::common::model::template_parameter &ct);

    /**
     * @brief Process template class parameters and arguments
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_base_params List of base class template parameters
     * @param template_args List of template arguments
     * @param template_instantiation Template class model to add template args
     * @param template_decl Base template declaration
     */
    void process_template_arguments(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        const clang::ArrayRef<clang::TemplateArgument> &template_args,
        clanguml::common::model::template_element &template_instantiation,
        const clang::TemplateDecl *template_decl);

    /**
     * @brief Process template arguments based on their type
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_instantiation Template class model to add template args
     * @param template_decl Base template declaration
     * @param arg Template argument
     * @param argument_index Argument index
     * @param argument Output list of arguments (can be more than one for
     *                 variadic parameters)
     */
    void argument_process_dispatch(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls,
        clanguml::common::model::template_element &template_instantiation,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg, size_t argument_index,
        std::vector<template_parameter> &argument);

    /**
     * @brief Process `clang::TemplateArgument::Expression`
     *
     * @note The template argument is a pack expansion of a template name that
     * was provided for a template template parameter.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    template_parameter process_expression_argument(
        const clang::TemplateArgument &arg);

    /**
     * @brief Process `clang::TemplateArgument::Integral`
     *
     * @note The template argument is an integral value stored in an
     * llvm::APSInt that was provided for an integral non-type template
     * parameter.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    template_parameter process_integral_argument(
        const clang::TemplateArgument &arg);

#if LLVM_VERSION_MAJOR > 17
    /**
     * @brief Process `clang::TemplateArgument::StructuralValue`
     *
     * @note The template argument is a non-type template argument that can't be
     *       represented by the special-case Declaration, NullPtr, or Integral
     *       forms.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    template_parameter process_structural_argument(
        const clang::TemplateArgument &arg);
#endif

    /**
     * @brief Process `clang::TemplateArgument::NullPtr`
     *
     * @note The template argument is a null pointer or null pointer to member
     * that was provided for a non-type template parameter.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    template_parameter process_nullptr_argument(
        const clang::TemplateArgument &arg);

    /**
     * @brief Process `clang::TemplateArgument::Null`
     *
     * @note Represents an empty template argument, e.g., one that has not
     * been deduced.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    template_parameter process_null_argument(
        const clang::TemplateArgument &arg);

    /**
     * @brief Process `clang::TemplateArgument::Pack`
     *
     * @note The template argument is actually a parameter pack. Arguments are
     * stored in the Args struct.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    std::vector<template_parameter> process_pack_argument(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls,
        clanguml::common::model::template_element &template_instantiation,
        const clang::TemplateDecl *base_template_decl,
        const clang::TemplateArgument &arg, size_t argument_index,
        std::vector<template_parameter> &argument);

    /**
     * @brief Process `clang::TemplateArgument::Type`
     *
     * @note The template argument is a type.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    template_parameter process_type_argument(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls,
        const clang::TemplateDecl *base_template_decl, clang::QualType type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Process `clang::TemplateArgument::Template`
     *
     * @note The template argument is a template name that was provided for a
     * template template parameter.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    common::model::template_parameter process_template_argument(
        const clang::TemplateArgument &arg);

    /**
     * @brief Process `clang::TemplateArgument::TemplateExpansion`
     *
     * @note The template argument is a pack expansion of a template name that
     * was provided for a template template parameter.
     *
     * @param arg Template argument
     * @return Return template argument model
     */
    common::model::template_parameter process_template_expansion(
        const clang::TemplateArgument &arg);

    /**
     * @brief Try to process template type argument as function template
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @param argument_index Argument index
     * @return Function template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_function_prototype(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Try to process template type argument as array
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @param argument_index Argument index
     * @return Array template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_array(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Try to process template type argument as specialization type
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @param argument_index Argument index
     * @return Template specialization template argument if succeeds,
     *         or std::nullopt
     */
    std::optional<template_parameter> try_as_template_specialization_type(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Try to process template type argument as template parameter
     *
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @return Template parameter if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_template_parm_type(
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type);

    /**
     * @brief Try to process template type argument as lambda
     *
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @return Lambda template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_lambda(const clang::NamedDecl *cls,
        const clang::TemplateDecl *template_decl, clang::QualType &type);

    /**
     * @brief Try to process template type argument as record type
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @param argument_index Argument index
     * @return Record type template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_record_type(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Try to process template type argument as enum type
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @return Enum type template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_enum_type(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation);

    /**
     * @brief Try to process template type argument as builtin type
     *
     * @param parent Optional class in which this template is contained
     * @param type Template type
     * @param template_decl Base template declaration
     * @return Builtin type template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_builtin_type(
        std::optional<clanguml::common::model::template_element *> &parent,
        clang::QualType &type, const clang::TemplateDecl *template_decl);

    /**
     * @brief Try to process template type argument as member pointer type
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @param argument_index Argument index
     * @return Member pointer type template argument if succeeds,
     *         or std::nullopt
     */
    std::optional<template_parameter> try_as_member_pointer(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Try to process template type argument as `decltype()` type
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @param argument_index Argument index
     * @return `decltype()` type template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_decl_type(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Try to process template type argument as typedef/using type
     *
     * @param parent Optional class in which this template is contained
     * @param cls Template class specialization declaration
     * @param template_decl Base template declaration
     * @param type Template type
     * @param template_instantiation Template class model
     * @param argument_index Argument index
     * @return Typedef type template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_typedef_type(
        std::optional<clanguml::common::model::template_element *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type,
        clanguml::common::model::template_element &template_instantiation,
        size_t argument_index);

    /**
     * @brief Remove types context (e.g. const or reference qualifiers)
     *
     * This method removes all const and reference/pointer qualifiers from
     * `type`, adds them to the template parameter model `tp` and returns
     * a type without context.
     *
     * @param type Type to remove context from
     * @param tp Template model to add context to
     * @return Type without context
     */
    clang::QualType consume_context(
        clang::QualType type, template_parameter &tp) const;

    /**
     * @brief Try to find additional relationships in unexposed parameters
     *
     * Sometimes, Clang will report a template parameter as unexposed, which
     * means all we get a is a string representation of the type, sometimes
     * with template parameter names replaced with `type-parameter-X-Y`
     * string.
     *
     * This method tries to find any type names, which might be relevant for
     * the diagram relationships.
     *
     * @param ct Template argument model
     * @param relationships List of discovered relationships
     * @return True, if any relationships were found
     */
    bool find_relationships_in_unexposed_template_params(
        const template_parameter &ct, found_relationships_t &relationships);

    void find_instantiation_relationships(
        common::model::template_element &template_instantiation, eid_t id,
        const std::string &qualified_name) const;

    /**
     * @brief Get reference to Clang AST to clang-uml id mapper
     *
     * @return Reference to Clang AST to clang-uml id mapper
     */
    common::visitor::ast_id_mapper &id_mapper();

    /**
     * @brief Get reference to the current source manager
     *
     * @return Reference to the current source manager
     */
    clang::SourceManager &source_manager() const;

private:
    // Reference to the output diagram model
    clanguml::common::model::diagram &diagram_;

    // Reference to diagram config
    const clanguml::config::diagram &config_;

    common::visitor::ast_id_mapper &id_mapper_;

    clang::SourceManager &source_manager_;

    VisitorT &visitor_;
};

template <typename VisitorT>
template_builder<VisitorT>::template_builder(
    clanguml::common::model::diagram &diagram_,
    const clanguml::config::diagram &config_, VisitorT &visitor)
    : diagram_{diagram_}
    , config_{config_}
    , id_mapper_{visitor.id_mapper()}
    , source_manager_{visitor.source_manager()}
    , visitor_{visitor}
{
}

template <typename VisitorT>
common::model::diagram &template_builder<VisitorT>::diagram()
{
    return diagram_;
}

template <typename VisitorT>
const config::diagram &template_builder<VisitorT>::config() const
{
    return config_;
}

template <typename VisitorT>
const namespace_ &template_builder<VisitorT>::using_namespace() const
{
    return config_.using_namespace();
}

template <typename VisitorT>
common::visitor::ast_id_mapper &template_builder<VisitorT>::id_mapper()
{
    return id_mapper_;
}

template <typename VisitorT>
clang::SourceManager &template_builder<VisitorT>::source_manager() const
{
    return source_manager_;
}

template <typename VisitorT>
bool template_builder<VisitorT>::simplify_system_template(
    template_parameter &ct, const std::string &full_name) const
{
    auto simplified = config().simplify_template_type(full_name);

    if (simplified != full_name) {
        ct.set_type(simplified);
        ct.set_id(common::to_id(simplified));
        ct.clear_params();
        return true;
    }

    return false;
}

template <typename VisitorT>
void template_builder<VisitorT>::build_from_template_declaration(
    clanguml::common::model::template_trait &template_model,
    const clang::TemplateDecl &template_declaration,
    common::optional_ref<common::model::element> templated_element)
{
    LOG_DBG("Processing {} template parameters...",
        common::get_qualified_name(template_declaration));

    if (template_declaration.getTemplateParameters() == nullptr)
        return;

    for (const auto *parameter :
        *template_declaration.getTemplateParameters()) {
        if (clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter) !=
            nullptr) {
            const auto *template_type_parameter =
                clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter);

            std::optional<std::string> default_arg;
            if (template_type_parameter->hasDefaultArgument()) {
                default_arg =
                    template_type_parameter->getDefaultArgument().getAsString();
            }

            auto parameter_name = template_type_parameter->getNameAsString();
            if (parameter_name.empty())
                parameter_name = "typename";

            auto ct = template_parameter::make_template_type(parameter_name,
                default_arg, template_type_parameter->isParameterPack());

            if constexpr (std::is_same_v<typename VisitorT::diagram_t,
                              clanguml::class_diagram::model::diagram>) {
                if (template_type_parameter->getTypeConstraint() != nullptr) {
                    util::if_not_null(
                        template_type_parameter->getTypeConstraint()
                            ->getNamedConcept(),
                        [this, &ct, &templated_element](
                            const clang::ConceptDecl *named_concept) mutable {
                            ct.set_concept_constraint(
                                named_concept->getQualifiedNameAsString());
                            if (templated_element &&
                                visitor_.should_include(named_concept)) {
                                templated_element.value().add_relationship(
                                    {relationship_t::kConstraint,
                                        id_mapper()
                                            .get_global_id(
                                                eid_t{named_concept->getID()})
                                            .value(),
                                        model::access_t::kNone,
                                        ct.name().value()});
                            }
                        });
                }
            }
            else {
                (void)templated_element; // NOLINT
            }

            template_model.add_template(std::move(ct));
        }
        else if (clang::dyn_cast_or_null<clang::NonTypeTemplateParmDecl>(
                     parameter) != nullptr) {
            const auto *template_nontype_parameter =
                clang::dyn_cast_or_null<clang::NonTypeTemplateParmDecl>(
                    parameter);

            std::optional<std::string> default_arg;

            if (template_nontype_parameter->hasDefaultArgument())
                default_arg = common::to_string(
                    template_nontype_parameter->getDefaultArgument());

            auto ct = template_parameter::make_non_type_template(
                template_nontype_parameter->getType().getAsString(),
                template_nontype_parameter->getNameAsString(), default_arg,
                template_nontype_parameter->isParameterPack());

            template_model.add_template(std::move(ct));
        }
        else if (clang::dyn_cast_or_null<clang::TemplateTemplateParmDecl>(
                     parameter) != nullptr) {
            const auto *template_template_parameter =
                clang::dyn_cast_or_null<clang::TemplateTemplateParmDecl>(
                    parameter);
            std::optional<std::string> default_arg;
            if (template_template_parameter->hasDefaultArgument()) {
                default_arg = common::to_string(
                    template_template_parameter->getDefaultArgument()
                        .getArgument());
            }
            auto ct = template_parameter::make_template_template_type(
                template_template_parameter->getNameAsString(), default_arg,
                template_template_parameter->isParameterPack());

            template_model.add_template(std::move(ct));
        }
        else {
            // pass
        }
    }
}

template <typename VisitorT>
void template_builder<VisitorT>::build_from_template_specialization_type(
    clanguml::common::model::template_element &template_instantiation,
    const clang::NamedDecl *cls,
    const clang::TemplateSpecializationType &template_type_decl,
    std::optional<clanguml::common::model::template_element *> parent)
{
    const auto *template_type_ptr = &template_type_decl;

    if (template_type_decl.isTypeAlias()) {
        if (const auto *tsp =
                template_type_decl.getAliasedType()
                    ->template getAs<clang::TemplateSpecializationType>();
            tsp != nullptr)
            template_type_ptr = tsp;
    }

    const auto &template_type = *template_type_ptr;

    template_instantiation.is_template(true);

    std::string full_template_specialization_name = common::to_string(
        template_type.desugar(),
        template_type.getTemplateName().getAsTemplateDecl()->getASTContext());

    auto *template_decl{template_type.getTemplateName().getAsTemplateDecl()};

    build(template_instantiation, cls, template_decl,
        template_type.template_arguments(), full_template_specialization_name,
        parent);
}

template <typename VisitorT>
void template_builder<VisitorT>::build(
    clanguml::common::model::template_element &template_instantiation,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    const clang::ArrayRef<clang::TemplateArgument> template_arguments,
    std::string full_template_specialization_name,
    std::optional<clanguml::common::model::template_element *> parent)
{
    //
    // Here we'll hold the template base class params to replace with the
    // instantiated values
    //
    std::deque<std::tuple</*parameter name*/ std::string, /* position */ int,
        /*is variadic */ bool>>
        template_base_params{};

    auto template_decl_qualified_name =
        template_decl->getQualifiedNameAsString();

    if (const auto *class_template_decl =
            clang::dyn_cast<clang::ClassTemplateDecl>(template_decl);
        (class_template_decl != nullptr) &&
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
        template_arguments, template_instantiation, template_decl);

    if constexpr (std::is_same_v<typename VisitorT::diagram_t,
                      class_diagram::model::diagram>) {
        find_instantiation_relationships(template_instantiation,
            eid_t{template_decl->getID()}, full_template_specialization_name);
    }

    template_instantiation.set_id(
        common::to_id(template_instantiation.full_name(false)));

    visitor_.set_source_location(*cls, template_instantiation);
}

template <typename VisitorT>
void template_builder<VisitorT>::build_from_class_template_specialization(
    clanguml::common::model::template_element &template_instantiation,
    const clang::ClassTemplateSpecializationDecl &template_specialization,
    std::optional<clanguml::common::model::template_element *> parent)
{
    //
    // Here we'll hold the template base params to replace with the
    // instantiated values
    //
    std::deque<std::tuple</*parameter name*/ std::string, /* position */ int,
        /*is variadic */ bool>>
        template_base_params{};

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

    if constexpr (std::is_same_v<typename VisitorT::diagram_t,
                      class_diagram::model::diagram>) {
        find_instantiation_relationships(template_instantiation,
            eid_t{template_specialization.getID()}, qualified_name);
    }

    visitor_.set_source_location(*template_decl, template_instantiation);
}

template <typename VisitorT>
void template_builder<VisitorT>::find_instantiation_relationships(
    common::model::template_element &template_instantiation, eid_t id,
    const std::string &qualified_name) const
{
    visitor_.find_instantiation_relationships(
        template_instantiation, qualified_name, id);
}

template <typename VisitorT>
void template_builder<VisitorT>::process_template_arguments(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls,
    std::deque<std::tuple<std::string, int, bool>> &template_base_params,
    const clang::ArrayRef<clang::TemplateArgument> &template_args,
    clanguml::common::model::template_element &template_instantiation,
    const clang::TemplateDecl *template_decl)
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

        if constexpr (std::is_same_v<typename VisitorT::diagram_t,
                          class_diagram::model::diagram>) {
            // In case any of the template arguments are base classes, add
            // them as parents of the current template instantiation class
            if (!template_base_params.empty()) {
                variadic_params = add_base_classes(template_instantiation,
                    template_base_params, arg_index, variadic_params,
                    arguments.front());
            }
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

    // Update id
    template_instantiation.set_id(
        common::to_id(template_instantiation.full_name(false)));
}

template <typename VisitorT>
void template_builder<VisitorT>::argument_process_dispatch(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls,
    clanguml::common::model::template_element &template_instantiation,
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
#if LLVM_VERSION_MAJOR > 17
    case clang::TemplateArgument::StructuralValue:
        break;
#endif
    }
}

template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_template_argument(
    const clang::TemplateArgument &arg)
{
    auto arg_name = common::to_string(arg);

    LOG_DBG("Processing template argument: {}", arg_name);

    return template_parameter::make_template_type(arg_name);
}

template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_template_expansion(
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

template <typename VisitorT>
clang::QualType template_builder<VisitorT>::consume_context(
    clang::QualType type, template_parameter &tp) const
{
    auto [unqualified_type, context] = common::consume_type_context(type);

    tp.deduced_context(std::move(context));

    return unqualified_type;
}

template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_type_argument(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType type,
    clanguml::common::model::template_element &template_instantiation,
    size_t argument_index)
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

template <typename VisitorT>
bool template_builder<VisitorT>::
    find_relationships_in_unexposed_template_params(
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

template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_integral_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::Integral);

    std::string result;
    llvm::raw_string_ostream ostream(result);
    arg.dump(ostream);

    return template_parameter::make_argument(result);
}

#if LLVM_VERSION_MAJOR > 17
template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_structural_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::StructuralValue);

    std::string result;
    llvm::raw_string_ostream ostream(result);
    arg.dump(ostream);

    return template_parameter::make_argument(result);
}
#endif

template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_null_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::Null);

    return template_parameter::make_argument("");
}

template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_nullptr_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::NullPtr);

    LOG_DBG("Processing nullptr argument: {}", common::to_string(arg));

    return template_parameter::make_argument("nullptr");
}

template <typename VisitorT>
template_parameter template_builder<VisitorT>::process_expression_argument(
    const clang::TemplateArgument &arg)
{
    assert(arg.getKind() == clang::TemplateArgument::Expression);
    return template_parameter::make_argument(common::get_source_text(
        arg.getAsExpr()->getSourceRange(), source_manager()));
}

template <typename VisitorT>
std::vector<template_parameter>
template_builder<VisitorT>::process_pack_argument(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls,
    clanguml::common::model::template_element &template_instantiation,
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

template <typename VisitorT>
std::optional<template_parameter>
template_builder<VisitorT>::try_as_member_pointer(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type,
    clanguml::common::model::template_element &template_instantiation,
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

template <typename VisitorT>
std::optional<template_parameter> template_builder<VisitorT>::try_as_array(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type,
    clanguml::common::model::template_element &template_instantiation,
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

template <typename VisitorT>
std::optional<template_parameter>
template_builder<VisitorT>::try_as_function_prototype(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type,
    clanguml::common::model::template_element &template_instantiation,
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

template <typename VisitorT>
std::optional<template_parameter> template_builder<VisitorT>::try_as_decl_type(
    std::optional<clanguml::common::model::template_element *> & /*parent*/,
    const clang::NamedDecl * /*cls*/,
    const clang::TemplateDecl * /*template_decl*/, clang::QualType &type,
    clanguml::common::model::template_element & /*template_instantiation*/,
    size_t /*argument_index*/)
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

template <typename VisitorT>
std::optional<template_parameter>
template_builder<VisitorT>::try_as_typedef_type(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl * /*cls*/,
    const clang::TemplateDecl * /*template_decl*/, clang::QualType &type,
    clanguml::common::model::template_element & /*template_instantiation*/,
    size_t /*argument_index*/)
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

template <typename VisitorT>
std::optional<template_parameter>
template_builder<VisitorT>::try_as_template_specialization_type(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
    clang::QualType &type,
    clanguml::common::model::template_element &template_instantiation,
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

    auto nested_template_instantiation =
        visitor_.create_element(nested_template_type->getTemplateName()
                                    .getAsTemplateDecl()
                                    ->getTemplatedDecl());
    build_from_template_specialization_type(*nested_template_instantiation, cls,
        *nested_template_type,
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

    argument.set_id(
        common::to_id(argument.to_string(using_namespace(), false)));

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
        visitor_.set_source_location(*cls, *nested_template_instantiation);
        visitor_.add_diagram_element(std::move(nested_template_instantiation));
    }

    return argument;
}

template <typename VisitorT>
std::optional<template_parameter>
template_builder<VisitorT>::try_as_template_parm_type(
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

    common::ensure_lambda_type_is_relative(config(), type_parameter_name);

    return argument;
}

template <typename VisitorT>
std::optional<template_parameter> template_builder<VisitorT>::try_as_lambda(
    const clang::NamedDecl *cls, const clang::TemplateDecl * /*template_decl*/,
    clang::QualType &type)
{
    auto type_name = common::to_string(type, &cls->getASTContext());

    if (type_name.find("(lambda at ") != 0)
        return {};

    LOG_DBG("Template argument is a lambda");

    auto argument = template_parameter::make_argument("");
    type = consume_context(type, argument);

    common::ensure_lambda_type_is_relative(config(), type_name);
    argument.set_type(type_name);

    return argument;
}

template <typename VisitorT>
std::optional<template_parameter>
template_builder<VisitorT>::try_as_record_type(
    std::optional<clanguml::common::model::template_element *> &parent,
    const clang::NamedDecl * /*cls*/, const clang::TemplateDecl *template_decl,
    clang::QualType &type,
    clanguml::common::model::template_element &template_instantiation,
    size_t /*argument_index*/)
{
    const auto *record_type =
        common::dereference(type)->getAs<clang::RecordType>();
    if (record_type == nullptr)
        return {};

    LOG_DBG("Template argument is a c++ record");

    auto argument = template_parameter::make_argument({});
    type = consume_context(type, argument);

    const auto type_name = config().simplify_template_type(
        common::to_string(type, template_decl->getASTContext()));

    argument.set_type(type_name);
    const auto type_id = common::to_id(type_name);

    argument.set_id(type_id);

    const auto *class_template_specialization =
        clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(
            record_type->getAsRecordDecl());

    if (class_template_specialization != nullptr) {
        auto tag_argument =
            visitor_.create_element(class_template_specialization);

        build_from_class_template_specialization(
            *tag_argument, *class_template_specialization);

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
                visitor_.add_diagram_element(std::move(tag_argument));
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

template <typename VisitorT>
std::optional<template_parameter> template_builder<VisitorT>::try_as_enum_type(
    std::optional<clanguml::common::model::template_element *> & /*parent*/,
    const clang::NamedDecl * /*cls*/, const clang::TemplateDecl *template_decl,
    clang::QualType &type,
    clanguml::common::model::template_element &template_instantiation)
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

template <typename VisitorT>
std::optional<template_parameter>
template_builder<VisitorT>::try_as_builtin_type(
    std::optional<clanguml::common::model::template_element *> & /*parent*/,
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

template <typename VisitorT>
bool template_builder<VisitorT>::add_base_classes(
    clanguml::common::model::template_element &tinst,
    std::deque<std::tuple<std::string, int, bool>> &template_base_params,
    int arg_index, bool variadic_params, const template_parameter &ct)
{
    bool add_template_argument_as_base_class = false;

    if (variadic_params) {
        add_template_argument_as_base_class = true;
    }
    else {
        auto [arg_name, index, is_variadic] = template_base_params.front();

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

        class_diagram::model::class_parent cp;
        cp.set_access(common::model::access_t::kPublic);
        cp.set_name(ct.to_string({}, false));
        cp.set_id(maybe_id.value());

        dynamic_cast<class_diagram::model::class_ &>(tinst).add_parent(
            std::move(cp));
    }

    return variadic_params;
}

} // namespace clanguml::common::visitor