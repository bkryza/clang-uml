/**
 * @file src/sequence_diagram/model/participant.h
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

#include "common/model/element.h"
#include "common/model/template_element.h"
#include "common/model/template_parameter.h"
#include "common/model/template_trait.h"

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

using clanguml::common::eid_t;
using clanguml::common::model::template_trait;

/**
 * @brief Base class for various types of sequence diagram participants
 */
struct participant : public common::model::template_element,
                     public common::model::stylable_element {
    using common::model::template_element::template_element;

    /**
     * @brief Enum representing stereotype of a participant
     */
    enum class stereotype_t {
        participant = 0,
        actor,
        boundary,
        control,
        entity,
        database,
        collections,
        queue
    };

    participant(const participant &) = delete;
    participant(participant &&) noexcept = delete;
    participant &operator=(const participant &) = delete;
    participant &operator=(participant &&) = delete;

    /**
     * Get the type name of the diagram element.
     *
     * @return Type name of the diagram element.
     */
    std::string type_name() const override { return "participant"; }

    /**
     * @brief Create a string representation of the participant
     *
     * @return Participant representation as string
     */
    virtual std::string to_string() const;

    stereotype_t stereotype_{stereotype_t::participant};
};

/**
 * @brief Sequence diagram participant representing a class.
 */
struct class_ : public participant {
public:
    class_(const common::model::namespace_ &using_namespace);

    class_(const class_ &) = delete;
    class_(class_ &&) noexcept = delete;
    class_ &operator=(const class_ &) = delete;
    class_ &operator=(class_ &&) = delete;

    /**
     * Get the type name of the diagram element.
     *
     * @return Type name of the diagram element.
     */
    std::string type_name() const override
    {
        if (is_lambda())
            return "lambda";

        return "class";
    }

    /**
     * @brief Check if class is a struct.
     *
     * @return True, if the class is declared as struct.
     */
    bool is_struct() const;

    /**
     * @brief Set whether the class is a struct.
     *
     * @param is_struct True, if the class is declared as struct
     */
    void is_struct(bool is_struct);

    /**
     * @brief Check if class is a template.
     *
     * @return True, if the class is a template.
     */
    bool is_template() const;

    /**
     * @brief Set whether the class is a template instantiation.
     *
     * @param is_template True, if the class is a template
     */
    void is_template(bool is_template);

    /**
     * @brief Check if class is a template instantiation.
     *
     * @return True, if the class is a template instantiation.
     */
    bool is_template_instantiation() const;

    /**
     * @brief Set whether the class is a template instantiation.
     *
     * @param is_template_instantiation True, if the class is a template
     *                                  instantiation.
     */
    void is_template_instantiation(bool is_template_instantiation);

    friend bool operator==(const class_ &l, const class_ &r);

    /**
     * Return elements full name.
     *
     * @return Fully qualified elements name.
     */
    std::string full_name(bool relative = true) const override;

    /**
     * Return elements full name but without namespace.
     *
     * @return Elements full name without namespace.
     */
    std::string full_name_no_ns() const override;

    /**
     * @brief Check if class is a abstract.
     *
     * @return True, if the class is abstract.
     */
    bool is_abstract() const;

    /**
     * @brief Check if class is a typedef/using alias.
     *
     * @return True, if the class is a typedef/using alias.
     */
    bool is_alias() const;

    /**
     * @brief Set whether the class is an alias
     *
     * @param alias True if the class is a typedef/using alias.
     */
    void is_alias(bool alias);

    /**
     * @brief Check if the class is lambda
     * @return
     */
    bool is_lambda() const;

    /**
     * @brief Set whether the class is a lambda.
     *
     * @param is_lambda True, if the class is a lambda
     */
    void is_lambda(bool is_lambda);

    void set_lambda_operator_id(eid_t id) { lambda_operator_id_ = id; }

    eid_t lambda_operator_id() const { return lambda_operator_id_; }

private:
    bool is_struct_{false};
    bool is_template_{false};
    bool is_template_instantiation_{false};
    bool is_alias_{false};
    bool is_lambda_{false};
    eid_t lambda_operator_id_{};

    std::string full_name_;
};

/**
 * @brief Participant mode representing a free function.
 */
struct function : public participant {
    enum class message_render_mode { full, abbreviated, no_arguments };

    function(const common::model::namespace_ &using_namespace);

    function(const function &) = delete;
    function(function &&) noexcept = delete;
    function &operator=(const function &) = delete;
    function &operator=(function &&) = delete;

    /**
     * Get the type name of the diagram element.
     *
     * @return Type name of the diagram element.
     */
    std::string type_name() const override { return "function"; }

    /**
     * Return elements full name.
     *
     * @return Fully qualified elements name.
     */
    std::string full_name(bool relative = true) const override;

    /**
     * Return elements full name but without namespace.
     *
     * @return Elements full name without namespace.
     */
    std::string full_name_no_ns() const override;

    /**
     * @brief Render function name as message label
     *
     * @param mode Function argument render mode
     * @return Message label
     */
    virtual std::string message_name(message_render_mode mode) const;

    /**
     * @brief Check if function is const
     *
     * @return True, if function is const
     */
    bool is_const() const;

    /**
     * @brief Set whether the function is const
     *
     * @param c True, if function is const
     */
    void is_const(bool c);

    /**
     * @brief Check, if the function has no return value
     *
     * @return True, if the function has no return value
     */
    bool is_void() const;

    /**
     * @brief Set whether the function has a return value
     *
     * @param v True, if the function has no return value
     */
    void is_void(bool v);

    /**
     * @brief Check, if the function is static
     *
     * @return True, if the function is static
     */
    bool is_static() const;

    /**
     * @brief Set whether the function is static
     *
     * @param v True, if the function is static
     */
    void is_static(bool s);

    /**
     * @brief Check, if the method is an operator
     *
     * @return True, if the method is an operator
     */
    bool is_operator() const;

    /**
     * @brief Set whether the method is an operator
     *
     * @param v True, if the method is an operator
     */
    void is_operator(bool o);

    /**
     * @brief Check, if a functions is a call to CUDA Kernel
     *
     * @return True, if the method is a CUDA kernel call
     */
    bool is_cuda_kernel() const;

    /**
     * @brief Set whether the method is a CUDA kernel call
     *
     * @param v True, if the method is a CUDA kernel call
     */
    void is_cuda_kernel(bool c);

    /**
     * @brief Check, if a functions is a call to CUDA device
     *
     * @return True, if the method is a CUDA device call
     */
    bool is_cuda_device() const;

    /**
     * @brief Set whether the method is a CUDA device call
     *
     * @param v True, if the method is a CUDA device call
     */
    void is_cuda_device(bool c);

    /**
     * @brief Set functions return type
     *
     * @param rt Return type
     */
    void return_type(const std::string &rt);

    /**
     * @brief Get function return type
     *
     * @return Return type
     */
    const std::string &return_type() const;

    /**
     * @brief Add a function parameter
     *
     * @note In sequence diagrams we don't care about relationships from
     * function or method parameters, so we don't need to model them in detail.
     *
     * @param a Function parameter label including name and type
     */
    void add_parameter(const std::string &a);

    /**
     * @brief Get the list of function parameters
     *
     * @return List of function parameters
     */
    const std::vector<std::string> &parameters() const;

private:
    bool is_const_{false};
    bool is_void_{false};
    bool is_static_{false};
    bool is_operator_{false};
    bool is_cuda_kernel_{false};
    bool is_cuda_device_{false};
    std::string return_type_;
    std::vector<std::string> parameters_;
};

/**
 * @brief Participant model representing a method
 */
struct method : public function {
    method(const common::model::namespace_ &using_namespace);

    method(const function &) = delete;
    method(method &&) noexcept = delete;
    method &operator=(const method &) = delete;
    method &operator=(method &&) = delete;

    /**
     * Get the type name of the diagram element.
     *
     * @return Type name of the diagram element.
     */
    std::string type_name() const override { return "method"; }

    /**
     * @brief Get method name
     * @return Method name
     */
    std::string method_name() const;

    /**
     * @brief Set method name
     *
     * @param name Method name
     */
    void set_method_name(const std::string &name);

    /**
     * @brief Get the participant PlantUML alias
     *
     * @todo This method does not belong here - refactor to PlantUML specific
     *       code.
     *
     * @return PlantUML alias for the participant to which this method belongs
     *         to.
     */
    std::string alias() const override;

    /**
     * @brief Set the id of the participant to which this method belongs to.
     *
     * @param id Id of the class to which this method belongs to
     */
    void set_class_id(eid_t id);

    /**
     * @brief Set full qualified name of the class
     *
     * @param name Name of the class including namespace
     */
    void set_class_full_name(const std::string &name);

    /**
     * @brief Get the class full name.
     *
     * @return Class full name
     */
    const auto &class_full_name() const;

    /**
     * Return elements full name.
     *
     * @return Fully qualified elements name.
     */
    std::string full_name(bool relative) const override;

    std::string message_name(message_render_mode mode) const override;

    /**
     * @brief Get the class id
     *
     * @return Class id
     */
    eid_t class_id() const;

    /**
     * @brief Create a string representation of the participant
     *
     * @return Participant representation as string
     */
    std::string to_string() const override;

    /**
     * @brief Check, if the method is a constructor
     *
     * @return True, if the method is a constructor
     */
    bool is_constructor() const;

    /**
     * @brief Set whether the method is a constructor
     *
     * @param v True, if the method is a constructor
     */
    void is_constructor(bool c);

    /**
     * @brief Check, if the method is defaulted
     *
     * @return True, if the method is defaulted
     */
    bool is_defaulted() const;

    /**
     * @brief Set whether the method is defaulted
     *
     * @param v True, if the method is defaulted
     */
    void is_defaulted(bool c);

    /**
     * @brief Check, if the method is an assignment operator
     *
     * @return True, if the method is an assignment operator
     */
    bool is_assignment() const;

    /**
     * @brief Set whether the method is an assignment operator
     *
     * @param v True, if the method is an assignment operator
     */
    void is_assignment(bool a);

private:
    eid_t class_id_{};
    std::string method_name_;
    std::string class_full_name_;
    bool is_constructor_{false};
    bool is_defaulted_{false};
    bool is_assignment_{false};
};

/**
 * @brief Participant model representing a function template.
 */
struct function_template : public function {
    function_template(const common::model::namespace_ &using_namespace);

    function_template(const function_template &) = delete;
    function_template(function_template &&) noexcept = delete;
    function_template &operator=(const function_template &) = delete;
    function_template &operator=(function_template &&) = delete;

    /**
     * Get the type name of the diagram element.
     *
     * @return Type name of the diagram element.
     */
    std::string type_name() const override { return "function_template"; }

    /**
     * Return elements full name.
     *
     * @return Fully qualified elements name.
     */
    std::string full_name(bool relative = true) const override;

    /**
     * Return elements full name but without namespace.
     *
     * @return Elements full name without namespace.
     */
    std::string full_name_no_ns() const override;

    /**
     * @brief Render function name as message label
     *
     * @param mode Function argument render mode
     * @return Message label
     */
    std::string message_name(message_render_mode mode) const override;
};
} // namespace clanguml::sequence_diagram::model
