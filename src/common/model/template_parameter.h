/**
 * @file src/common/model/template_parameter.h
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

#include "common/model/enums.h"
#include "common/model/namespace.h"
#include "common/types.h"

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace clanguml::common::model {

using clanguml::common::eid_t;

/**
 * Type of template parameter or argument.
 */
enum class template_parameter_kind_t {
    template_type,          /*!< Template type, e.g. <typename T> */
    template_template_type, /*!< Template template type, e.g. <typename <> T> */
    non_type_template,      /*!< Non type template parameter, e.g. <int N> */
    argument,               /*!< Template argument, e.g. <int> */
    concept_constraint      /*!< Concept constraint, e.g. <std::integral T> */
};

std::string to_string(template_parameter_kind_t k);

/**
 * Enum representing type of reference or pointer type qualifier
 */
enum class rpqualifier {
    kLValueReference, /*!< l-value reference (e.g. &) */
    kRValueReference, /*!< r-value reference (e.g. &&) */
    kPointer,         /*!< pointer (e.g *) */
    kNone             /*!< No qualifier */
};

/**
 * This struct manages a single level of template deduced context, e.g.
 * & or const*
 */
struct context {
    bool is_const{false};
    bool is_volatile{false};
    bool is_ref_const{false};
    bool is_ref_volatile{false};
    rpqualifier pr{rpqualifier::kNone};

    std::string to_string() const;

    bool operator==(const context &rhs) const;
    bool operator!=(const context &rhs) const;
};

/**
 * @brief Represents template parameter, template arguments or concept
 *        constraints
 *
 * This class can represent both template parameter and template arguments,
 * including variadic parameters and instantiations with
 * nested templates
 */
class template_parameter {
public:
    /**
     * @brief Build template type parameter
     *
     * @param name Name of template parameter (e.g. T)
     * @param default_value Default value of the parameter if any
     * @param is_variadic Whether the template parameter is variadic
     * @return template_parameter instance
     */
    static template_parameter make_template_type(const std::string &name,
        const std::optional<std::string> &default_value = {},
        bool is_variadic = false);

    /**
     * @brief Build template template parameter type
     *
     * @param name Name of template template parameter
     * @param default_value Default value of the parameter if any
     * @param is_variadic Whether the template parameter is variadic
     * @return template_parameter instance
     */
    static template_parameter make_template_template_type(
        const std::string &name,
        const std::optional<std::string> &default_value = {},
        bool is_variadic = false);

    /**
     * @brief Build non-type template parameter
     * @param type Type of non-type parameter (e.g. int)
     * @param name Name of parameter
     * @param default_value Default value of the parameter if any
     * @param is_variadic Whether the template parameter is variadic
     * @return template_parameter instance
     */
    static template_parameter make_non_type_template(const std::string &type,
        const std::optional<std::string> &name,
        const std::optional<std::string> &default_value = {},
        bool is_variadic = false);

    /**
     * @brief Build template argument
     *
     * @param type Type of template argument
     * @param default_value Default value of the parameter if any
     * @return template_parameter instance
     */
    static template_parameter make_argument(const std::string &type,
        const std::optional<std::string> &default_value = {});

    /**
     * @brief Build template argument with unexposed type
     *
     * This method is used to build template argument from an unexposed string,
     * i.e. when Clang just returns a string for the template argument type
     * instead of actual type AST.
     *
     * @param type String representation of the type
     * @param default_value Default value of the parameter if any
     * @return template_parameter instance
     */
    static template_parameter make_unexposed_argument(const std::string &type,
        const std::optional<std::string> &default_value = {});

    /**
     * Set the type of template argument
     *
     * @param type Name of the type
     */
    void set_type(const std::string &type);

    /**
     * Get the type of template parameter if any
     *
     * @return Optional name of type
     */
    std::optional<std::string> type() const;

    /**
     * Set unique id for the template parameter or argument
     *
     * @param id Id of parameter
     */
    void set_id(const eid_t &id) { id_ = id; }

    /**
     * Get id of the template parameter
     *
     * @return Id of the template parameter
     */
    const std::optional<eid_t> &id() const { return id_; }

    /**
     * Set the name of the template parameter
     *
     * @param name Name of template parameter
     */
    void set_name(const std::string &name);

    /**
     * Get the name of template parameter, if any
     *
     * @return Name of template parameter
     */
    std::optional<std::string> name() const;

    /**
     * Set default value for template parameter
     *
     * @param value Default value
     */
    void set_default_value(const std::string &value);

    /**
     * Get the template parameters default value if any
     *
     * @return Default value
     */
    const std::optional<std::string> &default_value() const;

    /**
     * Set template parameters variadic status.
     *
     * @param is_variadic True, if template parameter is variadic
     */
    void is_variadic(bool is_variadic) noexcept;

    /**
     * Check whether template parameter is variadic
     *
     * @return True, if template parameter is variadic
     */
    bool is_variadic() const noexcept;

    /**
     * @brief Calculate the match between this and other parameter
     *
     * This method calculates to what degree *this matches the
     * `base_template_parameter` as it's less specific specialization.
     *
     * The higher the value, the more likely it is that *this is a
     * specialization of `base_template_parameter`.
     *
     * @todo This is not scientific - there probably is more strict way to
     *       calculate this...
     *
     * @see calculate_template_params_specialization_match()
     *
     * @param base_template_parameter
     * @return Specialization match value
     */
    int calculate_specialization_match(
        const template_parameter &base_template_parameter) const;

    /**
     * Whether this is a template parameter.
     *
     * @return True, if *this is a template parameter
     */
    bool is_template_parameter() const;

    /**
     * Set, whether this is a template parameter.
     *
     * @param is_template_parameter Template parameter status
     */
    void is_template_parameter(bool is_template_parameter);

    /**
     * Whether this is a template template parameter.
     *
     * @return True, if *this is a template template parameter
     */
    bool is_template_template_parameter() const;

    /**
     * Set, whether this is a template template parameter.
     *
     * @param is_template_parameter Template template parameter status
     */
    void is_template_template_parameter(bool is_template_template_parameter);

    bool is_unexposed() const;
    void set_unexposed(bool unexposed);

    /**
     * Set, whether this is a function template parameter.
     *
     * @param ft Function template parameter status
     */
    void is_function_template(bool ft);

    /**
     * Whether this is a function template parameter.
     *
     * @return True, if *this is a function template parameter
     */
    bool is_function_template() const;

    /**
     * Set, whether this is a member pointer template parameter.
     *
     * @param m Member pointer template parameter status
     */
    void is_member_pointer(bool m);

    /**
     * Whether this is a member pointer parameter.
     *
     * @return True if this is member pointer parameter.
     */
    bool is_member_pointer() const;

    /**
     * Set, whether this is a data template parameter.
     *
     * @param m Data pointer parameter status
     */
    void is_data_pointer(bool m);

    /**
     * Whether this is a data pointer parameter.
     *
     * @return True, if this is a data pointer parameter.
     */
    bool is_data_pointer() const;

    /**
     * Set, whether this is an array template parameter.
     *
     * @param m Array parameter status
     */
    void is_array(bool a);

    /**
     * Whether this is an array template parameter.
     *
     * @return True, if this is an array template parameter.
     */
    bool is_array() const;

    /**
     * Add a nested template parameter.
     *
     * @param ct Template parameter r-value.
     */
    void add_template_param(template_parameter &&ct);

    /**
     * Add a nested template parameter.
     *
     * @param ct Template parameter l-value.
     */
    void add_template_param(const template_parameter &ct);

    /**
     * Get the reference to all nested template parameters.
     *
     * @return
     */
    const std::vector<template_parameter> &template_params() const;

    /**
     * Erase all nested template parameters.
     */
    void clear_params() { template_params_.clear(); }

    /**
     * Does the template parameters deduced context contain any references
     * or pointers?
     *
     * @return True if any of the deduced context is a reference or pointer.
     */
    bool is_association() const;

    /**
     * Is this template argument or parameter a specialization of some more
     * generic template.
     *
     * @return True, if this is specialization of some more generic template
     *         parameter.
     */
    bool is_specialization() const;

    /**
     * @brief Whether this is the same type of specialization as other.
     *
     * This method is used for instance to check if 2 template parameters
     * are function templates.
     *
     * @param other Another template parameter or argument
     * @return True, if both template parameters are the same type of
     *         specialzations.
     */
    bool is_same_specialization(const template_parameter &other) const;

    /**
     * @brief Find all relationships in this and its nested templates
     *
     * @param nested_relationships Output to store found relationships
     * @param hint Provide hint for the found relationships
     * @param should_include Functor to determine whether nested template
     *                       parameter or argument should be considered
     * @return
     */
    bool find_nested_relationships(
        std::vector<std::pair<eid_t, common::model::relationship_t>>
            &nested_relationships,
        common::model::relationship_t hint,
        const std::function<bool(const std::string &full_name)> &should_include)
        const;

    /**
     * Set the concept constraint for this template parameter
     *
     * @param constraint
     */
    void set_concept_constraint(std::string constraint);

    /**
     * Get the name of the concept constraint for this parameter, if any
     *
     * @return Optional concept constraint name
     */
    const std::optional<std::string> &concept_constraint() const;

    /**
     * Get the kind of the template parameter or argument
     * @return Template parameter kind
     */
    template_parameter_kind_t kind() const;

    /**
     * Set the kind of the template parameter or argument.
     *
     * @param kind Kind of the template parameter
     */
    void set_kind(template_parameter_kind_t kind);

    /**
     * @brief Append a deduced context to the template parameter.
     *
     * @param q Deduced context
     */
    void push_context(const context &q);

    /**
     * Get the entire deduce context of the template parameter
     *
     * @return Deduced context of this template parameter
     */
    const std::deque<context> &deduced_context() const;

    /**
     * Set the deduced context for the template parameter
     *
     * @param c Deduced context.
     */
    void deduced_context(std::deque<context> c);

    /**
     * Set, whether the parameter is an ellipsis (...)
     *
     * @param e True, if parameter is an ellipsis
     */
    void is_ellipsis(bool e);

    /**
     * Check whether the parameter is an ellipsis
     *
     * @return True if the parameter is an ellipsis
     */
    bool is_ellipsis() const;

    /**
     * @brief Render the template_parameter into string
     *
     * This method renders the template parameter along with any of its
     * nested template parameters
     * @param using_namespace
     * @param relative
     * @param skip_qualifiers
     * @return
     */
    std::string to_string(
        const clanguml::common::model::namespace_ &using_namespace,
        bool relative, bool skip_qualifiers = false) const;

private:
    /**
     * This class should be only constructed using builder methods.
     */
    template_parameter() = default;

    std::string deduced_context_str() const;

    template_parameter_kind_t kind_{template_parameter_kind_t::template_type};

    /*! Represents the type of non-type template parameters e.g. 'int' or type
     * of template arguments
     */
    std::optional<std::string> type_;

    /*! The name of the parameter (e.g. 'T' or 'N') */
    std::optional<std::string> name_;

    /*! Default value of the template parameter */
    std::optional<std::string> default_value_;

    /*! Whether the template parameter is a regular template parameter. When
     * false, it is a non-type template parameter
     */
    bool is_template_parameter_{false};

    /*! Whether the template parameter is a template template parameter.
     *  Can only be true when is_template_parameter_ is true
     */
    bool is_template_template_parameter_{false};

    /*! Whether template argument is ellipsis (...) */
    bool is_ellipsis_{false};

    /*! Whether the template parameter is variadic */
    bool is_variadic_{false};

    /*! Whether the template is a function template (e.g. R(T)) */
    bool is_function_template_{false};

    /*! Whether the template */
    bool is_data_pointer_{false};

    /*! Whether the template is a member pointer (e.g. R(C::*)(int)) */
    bool is_member_pointer_{false};

    /*! Is template argument an array */
    bool is_array_{false};

    /*! Stores the template parameter/argument deduction context e.g. const& */
    std::deque<context> context_;

    /*! Stores optional fully qualified name of constraint for this template
     * parameter
     */
    std::optional<std::string> concept_constraint_;

    /*! Nested template parameters. If this is a function template, the first
     * element is the return type
     */
    std::vector<template_parameter> template_params_;

    std::optional<eid_t> id_;

    bool is_unexposed_{false};
};

/**
 * @brief Calculate the match between 2 template parameter lists
 *
 * The higher the value, the more likely it is that *this is a
 * specialization of `base_template_parameter`.
 *
 * @todo This is not scientific - there probably is more strict way to
 *       calculate this...
 *
 * @param base_template_parameter
 * @return Specialization match value
 */
int calculate_template_params_specialization_match(
    const std::vector<template_parameter> &specialization,
    const std::vector<template_parameter> &base_template);

} // namespace clanguml::common::model
