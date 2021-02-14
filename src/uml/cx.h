#pragma once

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

namespace clanguml {
namespace cx {

namespace detail {
std::string to_string(CXString &&cxs)
{
    std::string r{clang_getCString(cxs)};
    clang_disposeString(cxs);
    return r;
}
}

using detail::to_string;

class type {
public:
    type(CXType &&t)
        : m_type{std::move(t)}
    {
    }
    ~type() = default;

    std::string spelling() const
    {
        return to_string(clang_getTypeSpelling(m_type));
    }

    bool operator==(const type &b) const
    {
        return clang_equalTypes(m_type, b.get());
    }

    type canonical() const { return {clang_getCanonicalType(m_type)}; }

    bool is_const_qualified() const
    {
        return clang_isConstQualifiedType(m_type);
    }

    bool is_volatile_qualified() const
    {
        return clang_isVolatileQualifiedType(m_type);
    }

    bool is_restricted_qualified() const
    {
        return clang_isRestrictQualifiedType(m_type);
    }

    std::string typedef_name() const
    {
        return to_string(clang_getTypedefName(m_type));
    }

    type pointee_type() const { return {clang_getPointeeType(m_type)}; }

    /*
     *cursor type_declaration() const
     *{
     *    return {clang_getTypeDeclaration(m_type)};
     *}
     */

    /*
     *std::string type_kind_spelling() const
     *{
     *    return clang_getTypeKindSpelling(kind());
     *}
     */

    CXCallingConv calling_convention() const
    {
        return clang_getFunctionTypeCallingConv(m_type);
    }

    type result_type() const { return clang_getResultType(m_type); }

    int exception_specification_type() const
    {
        return clang_getExceptionSpecificationType(m_type);
    }

    int argument_type_count() const { return clang_getNumArgTypes(m_type); }

    type argument_type(int i) const { return {clang_getArgType(m_type, i)}; }

    bool is_function_variadic() const
    {
        return clang_isFunctionTypeVariadic(m_type);
    }

    bool is_pod() const { return clang_isPODType(m_type); }

    type element_type() const { return clang_getElementType(m_type); }

    long long element_count() const { return clang_getNumElements(m_type); }

    type array_element_type() const
    {
        return clang_getArrayElementType(m_type);
    }

    type named_type() const { return clang_Type_getNamedType(m_type); }

    CXTypeNullabilityKind nullability() const
    {
        return clang_Type_getNullability(m_type);
    }

    type class_type() const { return clang_Type_getClassType(m_type); }

    long long size_of() const { return clang_Type_getSizeOf(m_type); }

    type modified_type() const { return clang_Type_getModifiedType(m_type); }

    type value_type() const { return clang_Type_getValueType(m_type); }

    int template_arguments_count() const
    {
        return clang_Type_getNumTemplateArguments(m_type);
    }

    type template_argument_type(int i) const
    {
        return clang_Type_getTemplateArgumentAsType(m_type, i);
    }

    const CXType &get() const { return m_type; }

    CXRefQualifierKind cxxref_qualifier() const
    {
        return clang_Type_getCXXRefQualifier(m_type);
    }

private:
    CXType m_type;
};

class cursor {
public:
    cursor()
        : m_cursor{clang_getNullCursor()}
    {
    }

    cursor(CXCursor &&c)
        : m_cursor{std::move(c)}
    {
    }

    cursor(const CXCursor &c)
        : m_cursor{c}
    {
    }

    cursor(const cursor &c)
        : m_cursor{c.get()}
    {
    }

    ~cursor() = default;

    bool operator==(const cursor &b) const
    {
        return clang_equalCursors(m_cursor, b.get());
    }

    cx::type type() const { return {clang_getCursorType(m_cursor)}; }

    std::string display_name() const
    {
        return to_string(clang_getCursorDisplayName(m_cursor));
    }

    std::string spelling() const
    {
        return to_string(clang_getCursorSpelling(m_cursor));
    }

    std::string fully_qualified() const
    {
        std::list<std::string> res;
        cursor iterator{m_cursor};
        while (iterator.kind() != CXCursor_TranslationUnit) {
            auto name = iterator.spelling();
            if (!name.empty())
                res.push_front(iterator.spelling());
            iterator = iterator.semantic_parent();
        }

        return fmt::format("{}", fmt::join(res, "::"));
    }

    cursor referenced() const
    {
        return cx::cursor{clang_getCursorReferenced(m_cursor)};
    }

    cursor semantic_parent() const
    {
        return {clang_getCursorSemanticParent(m_cursor)};
    }

    cursor lexical_parent() const
    {
        return {clang_getCursorLexicalParent(m_cursor)};
    }

    CXCursorKind kind() const { return m_cursor.kind; }

    bool is_definition() const { return clang_isCursorDefinition(m_cursor); }

    bool is_declaration() const { return clang_isDeclaration(kind()); }

    bool is_invalid_declaration() const
    {
        return clang_isInvalidDeclaration(m_cursor);
    }

    CXSourceLocation location() const
    {
        return clang_getCursorLocation(m_cursor);
    }

    bool is_reference() const { return clang_isReference(kind()); }

    bool is_expression() const { return clang_isExpression(kind()); }

    bool is_statement() const { return clang_isStatement(kind()); }

    bool is_attribute() const { return clang_isAttribute(kind()); }

    bool has_attrs() const { return clang_Cursor_hasAttrs(m_cursor); }

    bool is_invalid() const { return clang_isInvalid(kind()); }

    bool is_translation_unit() const { return clang_isTranslationUnit(kind()); }

    bool is_preprocessing() const { return clang_isPreprocessing(kind()); }

    CXVisibilityKind visibitity() const
    {
        return clang_getCursorVisibility(m_cursor);
    }

    CXAvailabilityKind availability() const
    {
        return clang_getCursorAvailability(m_cursor);
    }

    std::string usr() const { return to_string(clang_getCursorUSR(m_cursor)); }

    const CXCursor &get() const { return m_cursor; }

private:
    CXCursor m_cursor;
};
}
}
