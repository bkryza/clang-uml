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

using found_relationships_t = std::vector<
    std::pair<clanguml::common::id_t, common::model::relationship_t>>;

class translation_unit_visitor;

/**
 * @brief Class responsible for building all kinds of templates from Clang AST.
 */
class template_builder {
public:
    /**
     * @brief Constructor.
     *
     * @param visitor Reference to class diagram translation_unit_visitor
     */
    template_builder(
        clanguml::class_diagram::visitor::translation_unit_visitor &visitor);

    /**
     * @brief Get reference to the current diagram model
     *
     * @return Reference to the current diagram model
     */
    class_diagram::model::diagram &diagram();

    /**
     * @brief Get reference to the current diagram configuration
     *
     * @return Reference to the current diagram configuration
     */
    const config::class_diagram &config() const;

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

    /**
     * @brief Basic template class build method
     *
     * @param cls Clang template declaration
     * @param template_type_decl Template specialization type
     * @param parent Optional class in which this template is contained
     * @return Created template class model
     */
    std::unique_ptr<clanguml::class_diagram::model::class_> build(
        const clang::NamedDecl *cls,
        const clang::TemplateSpecializationType &template_type_decl,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    /**
     * @brief Build template class from class template specialization decl
     *
     * @param template_specialization Class template specialization declaration
     * @param parent Optional class in which this template is contained
     * @return Created template class model
     */
    std::unique_ptr<clanguml::class_diagram::model::class_>
    build_from_class_template_specialization(
        const clang::ClassTemplateSpecializationDecl &template_specialization,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

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
    bool add_base_classes(clanguml::class_diagram::model::class_ &tinst,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        const clang::ArrayRef<clang::TemplateArgument> &template_args,
        model::class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls,
        const clang::TemplateDecl *base_template_decl, clang::QualType type,
        model::class_ &template_instantiation, size_t argument_index);

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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation);

    /**
     * @brief Try to process template type argument as builtin type
     *
     * @param parent Optional class in which this template is contained
     * @param type Template type
     * @param template_decl Base template declaration
     * @return Builtin type template argument if succeeds, or std::nullopt
     */
    std::optional<template_parameter> try_as_builtin_type(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation,
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
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const clang::NamedDecl *cls, const clang::TemplateDecl *template_decl,
        clang::QualType &type, class_ &template_instantiation,
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
        const template_parameter &ct,
        class_diagram::visitor::found_relationships_t &relationships);

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
    clanguml::class_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::class_diagram &config_;

    common::visitor::ast_id_mapper &id_mapper_;

    clang::SourceManager &source_manager_;

    clanguml::class_diagram::visitor::translation_unit_visitor &visitor_;
};

} // namespace clanguml::class_diagram::visitor