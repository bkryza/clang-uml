/**
 * @file src/class_diagram/visitor/translation_unit_visitor.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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
#include "class_diagram/model/objc_interface.h"
#include "common/model/enums.h"
#include "common/model/template_trait.h"
#include "common/visitor/template_builder.h"
#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::class_diagram::visitor {

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_member;
using clanguml::class_diagram::model::class_member_base;
using clanguml::class_diagram::model::class_method;
using clanguml::class_diagram::model::class_method_base;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::diagram;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::method_parameter;
using clanguml::class_diagram::model::objc_interface;
using clanguml::class_diagram::model::objc_member;
using clanguml::class_diagram::model::objc_method;
using clanguml::common::eid_t;
using clanguml::common::model::access_t;
using clanguml::common::model::diagram_element;
using clanguml::common::model::namespace_;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;
using clanguml::common::model::template_parameter;
using clanguml::common::model::template_trait;
using clanguml::common::visitor::found_relationships_t;
using clanguml::common::visitor::template_builder;

using visitor_specialization_t =
    common::visitor::translation_unit_visitor<clanguml::config::class_diagram,
        clanguml::class_diagram::model::diagram>;

/**
 * @brief Class diagram translation unit visitor
 *
 * This class implements the `clang::RecursiveASTVisitor` interface
 * for selected visitors relevant to generating class diagrams.
 */
class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor>,
      public visitor_specialization_t {
public:
    using visitor_specialization_t::config_t;
    using visitor_specialization_t::diagram_t;

    using template_builder_t = template_builder<translation_unit_visitor>;

    /**
     * @brief Constructor.
     *
     * @param sm Current source manager reference
     * @param diagram Diagram model
     * @param config Diagram configuration
     */
    explicit translation_unit_visitor(clang::SourceManager &sm,
        clanguml::class_diagram::model::diagram &diagram,
        const clanguml::config::class_diagram &config);

    /**
     * \defgroup Implementation of ResursiveASTVisitor methods
     * @{
     */
    bool shouldVisitTemplateInstantiations() const { return false; }

    bool shouldVisitImplicitCode() const { return false; }

    virtual bool VisitNamespaceDecl(clang::NamespaceDecl *ns);

    virtual bool VisitRecordDecl(clang::RecordDecl *D);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *d);

    virtual bool VisitTypedefDecl(clang::TypedefDecl *decl);

    virtual bool VisitEnumDecl(clang::EnumDecl *e);

    virtual bool VisitClassTemplateDecl(
        clang::ClassTemplateDecl *class_template_declaration);

    virtual bool VisitClassTemplateSpecializationDecl(
        clang::ClassTemplateSpecializationDecl *cls);

    virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *cls);

    virtual bool TraverseConceptDecl(clang::ConceptDecl *cpt);

    virtual bool VisitObjCCategoryDecl(clang::ObjCCategoryDecl *decl);

    virtual bool VisitObjCProtocolDecl(clang::ObjCProtocolDecl *decl);

    virtual bool VisitObjCInterfaceDecl(clang::ObjCInterfaceDecl *decl);
    /** @} */

    /**
     * @brief Finalize diagram model
     *
     * This method is called after the entire AST has been visited by this
     * visitor. It is used to perform necessary post processing on the diagram
     * (e.g. resolve translation unit local element ID's into global ID's based
     * on elements full names).
     */
    void finalize();

    /**
     * @brief Add class (or template class) to the diagram.
     *
     * @param c Class model
     */
    void add_class(std::unique_ptr<class_> &&c);

    /**
     * @brief Add enum to the diagram.
     *
     * @param e Enum model
     */
    void add_enum(std::unique_ptr<enum_> &&e);

    /**
     * @brief Add concept to the diagram.
     *
     * @param c Concept model
     */
    void add_concept(std::unique_ptr<concept_> &&c);

    void add_objc_interface(std::unique_ptr<objc_interface> &&c);

    void add_diagram_element(
        std::unique_ptr<common::model::template_element> element) override;

    std::unique_ptr<class_> create_element(const clang::NamedDecl *decl) const;

    void find_instantiation_relationships(
        common::model::template_element &template_instantiation_base,
        const std::string &full_name, eid_t templated_decl_id);

private:
    /**
     * @brief Create class element model from class declaration
     *
     * @param cls Class declaration
     * @return Class diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::class_>
    create_class_declaration(clang::CXXRecordDecl *cls);

    /**
     * @brief Create enum element model from enum (e.g. struct) declaration
     *
     * @param rec Enum declaration
     * @return Enum diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::enum_>
    create_enum_declaration(
        const clang::EnumDecl *enm, const clang::TypedefDecl *typedef_decl);

    /**
     * @brief Create class element model from record (e.g. struct) declaration
     *
     * @param rec Record declaration
     * @return Class diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::class_>
    create_record_declaration(clang::RecordDecl *rec);

    /**
     * @brief Create class element model from Objective-C protocol
     *
     * @param decl Objective-C protocol declaration
     * @return Class diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::objc_interface>
    create_objc_protocol_declaration(clang::ObjCProtocolDecl *decl);

    /**
     * @brief Create class element model from Objective-C interface
     *
     * @param decl Objective-C protocol declaration
     * @return Class diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::objc_interface>
    create_objc_interface_declaration(clang::ObjCInterfaceDecl *decl);

    /**
     * @brief Create class element model from Objective-C category
     *
     * @param decl Objective-C protocol declaration
     * @return Class diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::objc_interface>
    create_objc_category_declaration(clang::ObjCCategoryDecl *decl);

    /**
     * @brief Create concept element model from concept declaration
     * @param cpt Concept declaration
     * @return Concept diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::concept_>
    create_concept_declaration(clang::ConceptDecl *cpt);

    /**
     * @brief Process class declaration
     *
     * @param cls Class declaration
     * @param c Class diagram element returned from `create_class_declaration`
     */
    void process_class_declaration(const clang::CXXRecordDecl &cls,
        clanguml::class_diagram::model::class_ &c);

    /**
     * @brief Process Objective-C category declaration
     *
     * @param cls Objective-C category declaration
     * @param c Class diagram element returned from
     *          `create_objc_category_declaration`
     */
    void process_objc_category_declaration(
        const clang::ObjCCategoryDecl &cls, objc_interface &c);

    /**
     * @brief Process Objective-C protocol declaration
     *
     * @param cls Objective-C protocol declaration
     * @param c Class diagram element returned from
     *          `create_objc_protocol_declaration`
     */
    void process_objc_protocol_declaration(
        const clang::ObjCProtocolDecl &cls, objc_interface &c);

    /**
     * @brief Process Objective-C interface declaration
     *
     * @param cls Objective-C interface declaration
     * @param c Class diagram element returned from
     *          `create_objc_interface_declaration`
     */
    void process_objc_interface_declaration(
        const clang::ObjCInterfaceDecl &cls, objc_interface &c);

    /**
     * @brief Process class declaration bases (parents), if any
     *
     * @param cls Class declaration
     * @param c Class diagram element model
     */
    void process_class_bases(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c);

    /**
     * @brief Process class children elements (members and methods)
     *
     * @param cls Class declaration
     * @param c Class diagram element model
     */
    void process_class_children(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c);

    /**
     * @brief Process class or record data members
     * @param cls Class declaration
     * @param c Class diagram element model
     */
    void process_record_members(const clang::RecordDecl *cls, class_ &c);

    /**
     * @brief Process class template specialization/instantiation
     *
     * @param cls Class template specialization declaration
     * @return Class diagram element model
     */
    std::unique_ptr<clanguml::class_diagram::model::class_>
    process_template_specialization(
        clang::ClassTemplateSpecializationDecl *cls);

    /**
     * @brief Process template specialization children (members and methods)
     * @param cls Class template specialization declaration
     * @param c Class diagram element model
     */
    void process_template_specialization_children(
        const clang::ClassTemplateSpecializationDecl *cls, class_ &c);

    /**
     * @brief Process class method
     *
     * @param mf Method declaration
     * @param c Class diagram element model
     */
    void process_method(const clang::CXXMethodDecl &mf,
        clanguml::class_diagram::model::class_ &c);

    /**
     * @brief Process Objective-C method
     *
     * @param mf Method declaration
     * @param c Class diagram element model
     */
    void process_objc_method(
        const clang::ObjCMethodDecl &mf, objc_interface &c);

    /**
     * @brief Process class method properties
     * @param mf Method declaration
     * @param c Class diagram element model
     * @param method_name Method name
     * @param method Method model
     */
    void process_method_properties(const clang::CXXMethodDecl &mf,
        const class_ &c, const std::string &method_name,
        class_method &method) const;

    /**
     * @brief Process class template method
     *
     * @param mf Method declaration
     * @param c Class diagram element model
     */
    void process_template_method(const clang::FunctionTemplateDecl &mf,
        clanguml::class_diagram::model::class_ &c);

    /**
     * @brief Process class static data member
     *
     * @param field_declaration Static data member declaration
     * @param c Class diagram element model
     */
    void process_static_field(const clang::VarDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    /**
     * @brief Process class data member
     *
     * @param field_declaration Data member declaration
     * @param c Class diagram element model
     */
    void process_field(const clang::FieldDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    /**
     * @brief Process Objective-C data member
     *
     * @param field_declaration Data member declaration
     * @param c Class diagram element model
     */
    void process_objc_ivar(const clang::ObjCIvarDecl &ivar, objc_interface &c);

    /**
     * @brief Process Objective-C class base
     *
     * @param cls Objective-C interface declaration
     * @param c Class diagram element model
     */
    void process_objc_interface_base(
        const clang::ObjCInterfaceDecl &cls, objc_interface &c);

    /**
     * @brief Process function/method parameter
     *
     * @param param Parameter declaration
     * @param method Class method model
     * @param c Class diagram element model
     * @param template_parameter_names Ignored
     */
    void process_function_parameter(const clang::ParmVarDecl &param,
        class_method &method, class_ &c,
        const std::set<std::string> &template_parameter_names = {});

    /**
     * @brief Process Objective-C class method parameter
     *
     * @param param Parameter declaration
     * @param method Class method model
     * @param c Class diagram element model
     * @param template_parameter_names Ignored
     */
    void process_objc_method_parameter(const clang::ParmVarDecl &param,
        objc_method &method, objc_interface &c);

    /**
     * @brief Process class friend
     *
     * @param f Friend declaration
     * @param c Class diagram element model
     */
    void process_friend(const clang::FriendDecl &f, class_ &c);

    /**
     * @brief Find relationships in a specific type
     *
     * @param type Type to search for relationships
     * @param relationship_hint Default relationship type to infer from this
     *                          type
     * @return True, if any relationships were found
     */
    bool find_relationships(const clang::QualType &type,
        found_relationships_t & /*relationships*/,
        clanguml::common::model::relationship_t relationship_hint);

    /**
     * @brief Add relationships from relationship list to a diagram element
     * model
     *
     * This method takes a list of relationships whose originating element
     * is class `c` and adds them to it, ignoring any duplicates and skipping
     * relationships that should be excluded from the diagram.
     *
     * @param c Diagram element model
     * @param field Class member model
     * @param relationships List of found relationships
     * @param break_on_first_aggregation Stop adding relatinoships, after first
     *        aggregation is found
     */
    void add_relationships(diagram_element &c, const class_member_base &field,
        const found_relationships_t &relationships,
        bool break_on_first_aggregation = false);

    /**
     * @brief Process record parent element (e.g. for nested classes)
     *
     * This method handles nested classes or structs.
     *
     * @param cls Record declaration
     * @param c Class diagram element model
     * @param ns Package in the diagram to which the class `c` should belong
     */
    void process_record_parent(
        clang::RecordDecl *cls, class_ &c, const namespace_ &ns);

    /**
     * @brief Find relationships in function parameter
     *
     * @param c Class diagram element model
     * @param atsp `auto` type
     */
    void process_function_parameter_find_relationships_in_autotype(
        model::class_ &c, const clang::AutoType *atsp);

    /**
     * @brief Find relationships in concept constraint expression
     *
     * @param c Diagram element model (concept)
     * @param expr Concept constraint expression
     */
    void find_relationships_in_constraint_expression(
        clanguml::common::model::element &c, const clang::Expr *expr);

    /**
     * @brief Register incomplete forward declaration to be updated later
     */
    void add_incomplete_forward_declarations();

    /**
     * @brief Replace any AST local ids in diagram elements with global ones
     *
     * Not all elements global ids can be set in relationships during
     * traversal of the AST. In such cases, a local id (obtained from `getID()`)
     * and at after the traversal is complete, the id is replaced with the
     * global diagram id.
     */
    void resolve_local_to_global_ids();

    /**
     * @brief Process concept constraint requirements
     *
     * @param cpt Concept declaration
     * @param expr Requires expression
     * @param concept_model Concept diagram element model
     */
    void process_constraint_requirements(const clang::ConceptDecl *cpt,
        const clang::Expr *expr, model::concept_ &concept_model) const;

    /**
     * @brief Find concept specializations relationships
     *
     * @param c Concept element model
     * @param concept_specialization Concept specialization expression
     */
    void process_concept_specialization_relationships(common::model::element &c,
        const clang::ConceptSpecializationExpr *concept_specialization);

    /**
     * @brief Extract template contraint parameter name from raw source code
     *
     * @param concept_specialization Concept specialization expression
     * @param cpt Concept declaration
     * @param constrained_template_params Found constraint template param names
     * @param argument_index Argument index
     * @param type_name Type parameter name - used if extraction fails
     */
    void extract_constrained_template_param_name(
        const clang::ConceptSpecializationExpr *concept_specialization,
        const clang::ConceptDecl *cpt,
        std::vector<std::string> &constrained_template_params,
        size_t argument_index, std::string &type_name) const;

    /**
     * @brief Register already processed template class name
     *
     * @param qualified_name Fully qualified template class name
     */
    void add_processed_template_class(std::string qualified_name);

    /**
     * @brief Check if template class has already been processed
     *
     * @param qualified_name Fully qualified template class name
     * @return True, if template class has already been processed
     */
    bool has_processed_template_class(const std::string &qualified_name) const;

    /**
     * @brief Get template builder reference
     *
     * @return Reference to 'template_builder' instance
     */
    template_builder_t &tbuilder() { return template_builder_; }

    template <typename T>
    void process_record_parent_by_type(eid_t parent_id, class_ &c,
        namespace_ parent_ns, const clang::RecordDecl *decl);

    void find_record_parent_id(const clang::TagDecl *decl,
        std::optional<eid_t> &parent_id_opt, namespace_ &parent_ns) const;

    template_builder_t template_builder_;

    std::map<eid_t, std::unique_ptr<clanguml::class_diagram::model::class_>>
        forward_declarations_;

    std::map<int64_t /* local anonymous struct id */,
        std::tuple<std::string /* field name */, common::model::relationship_t,
            common::model::access_t,
            std::optional<size_t> /* destination_multiplicity */>>
        anonymous_struct_relationships_;

    std::map<const clang::EnumDecl *, const clang::TypedefDecl *>
        typedef_enum_decls_;

    /**
     * When visiting CXX records we need to know if they have already been
     * process in VisitClassTemplateDecl or
     * VisitClassTemplateSpecializationDecl. If yes, then we need to skip it
     *
     * @todo There must be a better way to do this...
     */
    std::set<std::string> processed_template_qualified_names_;
};

template <typename T>
void translation_unit_visitor::process_record_parent_by_type(eid_t parent_id,
    class_ &c, namespace_ parent_ns, const clang::RecordDecl *decl)
{
    // Here we have 2 options, either:
    //  - the parent is a regular C++ class/struct
    //  - the parent is a class template declaration/specialization
    auto parent_class = diagram().find<T>(parent_id);

    c.set_namespace(parent_ns);
    const auto cls_name = decl->getNameAsString();
    if (cls_name.empty()) {
        // Nested structs can be anonymous
        if (anonymous_struct_relationships_.count(decl->getID()) > 0) {
            const auto &[label, hint, access, destination_multiplicity] =
                anonymous_struct_relationships_[decl->getID()];

            c.set_name(parent_class.value().name() + "##" +
                fmt::format("({})", label));

            std::string destination_multiplicity_str{};
            if (destination_multiplicity.has_value()) {
                destination_multiplicity_str =
                    std::to_string(*destination_multiplicity);
            }

            parent_class.value().add_relationship(
                {hint, common::to_id(c.full_name(false)), access, label, "",
                    destination_multiplicity_str});
        }
        else
            c.set_name(parent_class.value().name() + "##" +
                fmt::format("(anonymous_{})", std::to_string(decl->getID())));
    }
    else {
        c.set_name(
            parent_class.value().name() + "##" + decl->getNameAsString());
    }

    c.set_id(common::to_id(c.full_name(false)));

    if (!(decl->getNameAsString().empty())) {
        // Don't add anonymous structs as contained in the class
        // as they are already added as aggregations
        c.add_relationship({relationship_t::kContainment, parent_id});
    }

    c.nested(true);
}
} // namespace clanguml::class_diagram::visitor
