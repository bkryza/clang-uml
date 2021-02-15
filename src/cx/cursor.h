#pragma once

#include "cx/type.h"

namespace clanguml {
namespace cx {

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
