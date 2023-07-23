#ifndef MIROIR_MIROIR_HPP
#define MIROIR_MIROIR_HPP

#include <cctype>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace miroir {

// wrapper around any custom node type
template <typename Node> struct NodeAccessor {
    // node iterator type
    // for sequence: contains children nodes directly
    // for map: contains key-value pairs
    using Iterator = void;

    // returns true if node exists
    static auto is_defined(const Node &node) -> bool;
    // returns true if node is explicitly typed (e.g. quoted for strings)
    static auto is_explicit(const Node &node) -> bool;

    // self-explanatory
    static auto is_scalar(const Node &node) -> bool;
    static auto is_sequence(const Node &node) -> bool;
    static auto is_map(const Node &node) -> bool;

    // returns child node by the key/index
    template <typename Key> static auto at(const Node &node, const Key &key) -> Node;

    // casts node to type T
    template <typename T> static auto as(const Node &node) -> T;
    // casts node to type T or returns a fallback value of the same type
    template <typename T> static auto as(const Node &node, const T &fallback) -> T;

    // returns tag of the node
    static auto tag(const Node &node) -> std::string;
    // returns string representation of the node
    static auto dump(const Node &node) -> std::string;

    // returns true if both nodes have the same content
    static auto equals(const Node &lhs, const Node &rhs) -> bool;
    // returns true if both nodes point to the same memory
    static auto is_same(const Node &lhs, const Node &rhs) -> bool;

    // returns number of children for map and sequence nodes
    static auto size(const Node &node) -> std::size_t;
    // returns beginning of the node iterator
    static auto begin(const Node &node) -> Iterator;
    // returns ending of the node iterator
    static auto end(const Node &node) -> Iterator;
};

enum class ErrorType {
    NodeNotFound,       // <path>: node not found
    InvalidValueType,   // <path>: expected value type: <type>
    InvalidValue,       // <path>: expected value: <value>
    MissingKeyWithType, // <path>: missing key with type: <type>
    UndefinedNode,      // <path>: undefined node
};

template <typename Node> struct Error {
    ErrorType type;
    std::string path;                                         // path of the node in the document
    std::variant<std::monostate, std::string, Node> expected; // expected type

    // errors that occurred during the validation of type variants
    std::vector<std::vector<Error<Node>>> variant_errors;

    // returns user-friendly error message
    // int max_depth - maximum depth of nested errors (0 = infinite depth, 1 = flat, etc.)
    auto description(int max_depth = 0) const -> std::string;
};

template <typename Node> class Validator {
  public:
    using Error = miroir::Error<Node>;
    using NodeAccessor = miroir::NodeAccessor<Node>;
    using TypeValidator = auto(*)(const Node &val) -> bool;

  public:
    explicit Validator(const Node &schema,
                       const std::map<std::string, TypeValidator> &type_validators = {});

    auto validate(const Node &doc) const -> std::vector<Error>;

  private:
    struct SchemaSettings {
        bool default_required;
        std::string optional_tag;
        std::string required_tag;
        std::string embed_tag;
        std::string variant_tag;
        std::string key_type_prefix;
        std::string generic_brackets;
        std::string generic_separator;
        std::string attribute_separator;
        bool ignore_attributes;
    };

    struct GenericType {
        std::string name;
        std::vector<std::string> args;
    };

    struct Context {
        std::string path;
        // note: prohibit assignment to a Node, since it may be a reference type, and assignment to
        // an old node may invalidate/replace internal memory of that node (at least the yaml-cpp
        // YAML::Node has this behavior)
        const std::variant<std::monostate, std::string, Node> expected;
        std::map<std::string, std::string> where;
        bool is_embed;

        explicit Context(const Node &expected)
            : path{"/"}, expected{expected}, where{}, is_embed{false} {}

        explicit Context(const Context &other,
                         const std::variant<std::monostate, std::string, Node> &expected)
            : path{other.path}, expected{expected}, where{other.where}, is_embed{other.is_embed} {}

        auto appending_path(const std::string &suffix) const -> Context;
        auto with_expected(const std::variant<std::monostate, std::string, Node> &expected) const
            -> Context;
        auto with_where(const std::map<std::string, std::string> &where) const -> Context;
        auto with_embed() const -> Context;
    };

  private:
    static auto schema_settings(const Node &schema) -> SchemaSettings;
    static auto schema_types(const Node &schema) -> std::map<std::string, Node>;
    static auto schema_root(const Node &schema) -> Node;

  private:
    auto make_error(ErrorType type, const Context &ctx,
                    const std::vector<std::vector<Error>> &variant_errors = {}) const -> Error;

    void validate(const Node &doc, const Node &schema, const Context &ctx,
                  std::vector<Error> &errors) const;

    void validate_type(const Node &doc, const std::string &type, const Context &ctx,
                       std::vector<Error> &errors) const;
    auto validate_type(const Node &doc, const std::string &type, const Context &ctx) const -> bool;

    void validate_scalar(const Node &doc, const Node &schema, const Context &ctx,
                         std::vector<Error> &errors) const;
    void validate_sequence(const Node &doc, const Node &schema, const Context &ctx,
                           std::vector<Error> &errors) const;
    void validate_map(const Node &doc, const Node &schema, const Context &ctx,
                      std::vector<Error> &errors) const;

    auto tag_is_optional(const std::string &tag) const -> bool;
    auto tag_is_embed(const std::string &tag) const -> bool;
    auto tag_is_variant(const std::string &tag) const -> bool;
    auto tag_is_required(const std::string &tag) const -> bool;

    auto find_node(const Node &map, const std::string &key) const -> std::optional<Node>;

    auto type_is_generic(const std::string &type) const -> bool;
    auto parse_generic_type(const std::string &type) const -> GenericType;
    auto make_generic_args(const GenericType &keys, const GenericType &vals,
                           const std::map<std::string, std::string> &where) const
        -> std::map<std::string, std::string>;

  private:
    const SchemaSettings m_settings;
    const std::map<std::string, Node> m_types;
    const Node m_root;

    std::map<std::string, TypeValidator> m_validators;

    mutable std::map<std::string, GenericType> m_generic_types_cache;
};

} // namespace miroir

#endif // ifndef MIROIR_MIROIR_HPP

#ifdef MIROIR_IMPLEMENTATION

#include <algorithm>
#include <limits>
#include <set>
#include <sstream>
#include <string_view>

// MIROIR_ASSERT macro
#ifndef MIROIR_ASSERT
#ifndef NDEBUG
#include <iostream>
#define MIROIR_ASSERT(cond, msg)                                                                   \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            /* todo: (c++20) use std::format */                                                    \
            std::cerr << "FATAL " << __FILE__ << ":" << __LINE__ << ": assertion failure ( "       \
                      << #cond << " )" << std::endl;                                               \
            std::cerr << "FATAL " << msg << std::endl;                                             \
            std::abort();                                                                          \
        }                                                                                          \
    } while (false)
#else // ifndef NDEBUG
#define MIROIR_ASSERT(cond, msg)
#endif // ifndef NDEBUG
#endif // ifndef MIROIR_ASSER

namespace miroir {

namespace impl {

/// Misc

[[noreturn]] inline void unreachable() {
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#ifdef __GNUC__ // GCC, Clang, ICC
    __builtin_unreachable();
#elif defined(_MSC_VER) // MSVC
    __assume(false);
#endif
}

/// Strings

auto string_is_prefixed(const std::string &str, const std::string &prefix) -> bool {
    return str.compare(0, prefix.size(), prefix) == 0;
}

auto string_trim_after(const std::string &str, char c) -> std::string_view {
    const std::string::size_type pos = str.find(c);

    if (pos != std::string::npos) {
        return std::string_view{str}.substr(0, pos);
    } else {
        return std::string_view{str};
    }
}

auto string_indent(const std::string &str) -> std::string {
    std::istringstream iss{str};

    std::string line;
    line.reserve(str.size());

    std::string result;
    line.reserve(str.size());

    while (std::getline(iss, line, '\n')) {
        // todo: (c++20) use std::format
        result += "\n\t\t" + line;
    }

    return result;
}

/// Nodes

template <typename Node>
auto nodes_contains_node(const std::vector<Node> &nodes, const Node &target) -> bool {
    for (const Node &node : nodes) {
        if (NodeAccessor<Node>::is_same(node, target)) {
            return true;
        }
    }

    return false;
}

/// Errors

template <typename Node>
auto dump_expected(const std::variant<std::monostate, std::string, Node> &expected) -> std::string {
    using NodeAccessor = NodeAccessor<Node>;

    if (std::holds_alternative<std::monostate>(expected)) {
        return "";
    } else if (std::holds_alternative<std::string>(expected)) {
        return std::get<std::string>(expected);
    }

    const Node node = std::get<Node>(expected);

    if (!NodeAccessor::is_sequence(node) || NodeAccessor::size(node) <= 1) {
        return NodeAccessor::dump(node);
    }

    std::string str = "one of";

    for (auto it = NodeAccessor::begin(node); it != NodeAccessor::end(node); ++it) {
        // todo: (c++20) use std::format
        str += "\n\t- " + NodeAccessor::dump(*it);
    }

    return str;
}

template <typename Node>
auto dump_variant_errors(const std::vector<std::vector<Error<Node>>> &variant_errors, int max_depth)
    -> std::string {

    if (max_depth == 0) {
        return "";
    }

    std::string result;

    for (std::size_t i = 0; i < variant_errors.size(); ++i) {
        // todo: (c++20) use std::format
        result += "\n\t* failed variant " + std::to_string(i) + ":";

        for (const Error<Node> &err : variant_errors[i]) {
            result += impl::string_indent(err.description(max_depth));
        }
    }

    return result;
}

template <typename Node>
void filter_undefined_node_errors(std::vector<Error<Node>> &errors, std::size_t embed_count) {
    using Error = Error<Node>;

    // remove errors that are not present in all embedded nodes
    errors.erase(std::remove_if(errors.begin(), errors.end(),
                                [&errors, embed_count](const Error &remove_err) -> bool {
                                    if (remove_err.type != ErrorType::UndefinedNode) {
                                        return false;
                                    }

                                    const std::size_t count = std::count_if(
                                        errors.cbegin(), errors.cend(),
                                        [&remove_err](const Error &count_err) -> bool {
                                            return count_err.type == remove_err.type &&
                                                   count_err.path == remove_err.path;
                                        });

                                    return count < embed_count + 1;
                                }),
                 errors.end());

    // remove duplicate errors preserving the order and keeping only the last occurrence of error
    std::set<std::pair<ErrorType, std::string>> visited_errors;
    errors.erase(errors.begin(),
                 std::stable_partition(errors.rbegin(), errors.rend(),
                                       [&visited_errors](const Error &err) -> bool {
                                           if (err.type != ErrorType::UndefinedNode) {
                                               return true;
                                           }

                                           if (visited_errors.count({err.type, err.path}) > 0) {
                                               return false;
                                           }

                                           visited_errors.insert({err.type, err.path});
                                           return true;
                                       })
                     .base());
}

/// Built-in validators

template <typename Node> auto node_is_integer(const Node &node) -> bool {
    using NodeAccessor = NodeAccessor<Node>;

    if (!NodeAccessor::is_scalar(node)) {
        return false;
    }

    const std::string val = NodeAccessor::template as<std::string>(node);
    std::istringstream iss{val};

    long long integer;
    iss >> integer;

    return iss.eof() && !iss.fail();
}

template <typename Node> auto node_is_number(const Node &node) -> bool {
    using NodeAccessor = NodeAccessor<Node>;

    if (!NodeAccessor::is_scalar(node)) {
        return false;
    }

    const std::string val = NodeAccessor::template as<std::string>(node);
    std::istringstream iss{val};

    double number;
    iss >> number;

    return iss.eof() && !iss.fail();
}

template <typename Node> auto node_is_boolean(const Node &node) -> bool {
    using NodeAccessor = NodeAccessor<Node>;

    if (!NodeAccessor::is_scalar(node)) {
        return false;
    }

    static const struct {
        std::string trueval, falseval;
    } boolvals[] = {
        {"y", "n"},
        {"yes", "no"},
        {"true", "false"},
        {"on", "off"},
    };

    const std::string val = NodeAccessor::template as<std::string>(node);

    for (const auto &boolval : boolvals) {
        if (val == boolval.trueval || val == boolval.falseval) {
            return true;
        }
    }

    return false;
}

template <typename Node> auto node_is_string(const Node &node) -> bool {
    using NodeAccessor = NodeAccessor<Node>;

    if (!NodeAccessor::is_scalar(node)) {
        return false;
    }

    // value is explicitly quoted
    if (NodeAccessor::is_explicit(node)) {
        return true;
    }

    return !impl::node_is_integer(node) && !impl::node_is_number(node) &&
           !impl::node_is_boolean(node);
}

} // namespace impl

/// Error

template <typename Node> auto Error<Node>::description(int max_depth) const -> std::string {
    MIROIR_ASSERT(max_depth >= 0, "max_depth is negative");

    if (max_depth == 0) {
        max_depth = std::numeric_limits<int>::max();
    }

    --max_depth;

    // todo: (c++20) use std::format
    switch (type) {
    case ErrorType::NodeNotFound:
        return path + ": node not found";
    case ErrorType::InvalidValueType:
        return path + ": expected value type: " + impl::dump_expected(expected) +
               impl::dump_variant_errors(variant_errors, max_depth);
    case ErrorType::InvalidValue:
        return path + ": expected value: " + impl::dump_expected(expected);
    case ErrorType::MissingKeyWithType:
        return path + ": missing key with type: " + impl::dump_expected(expected);
    case ErrorType::UndefinedNode:
        return path + ": undefined node";
    }

    MIROIR_ASSERT(false, "invalid error type: " << static_cast<int>(type));
    // todo: (c++23) use std::unreachable
    impl::unreachable();
}

/// Context

template <typename Node>
auto Validator<Node>::Context::appending_path(const std::string &suffix) const -> Context {
    Context ctx = *this;
    // todo: (c++20) use std::format
    ctx.path = path != "/" ? path + "." + suffix : path + suffix;
    ctx.is_embed = false; // reset is_embed field when we're going deeper
    return ctx;
}

template <typename Node>
auto Validator<Node>::Context::with_expected(
    const std::variant<std::monostate, std::string, Node> &expected) const -> Context {

    // see a note for the expected field in the Context struct definition
    return Context{*this, expected};
}

template <typename Node>
auto Validator<Node>::Context::with_where(const std::map<std::string, std::string> &where) const
    -> Context {

    Context ctx = *this;
    ctx.where = where;
    return ctx;
}

template <typename Node> auto Validator<Node>::Context::with_embed() const -> Context {
    Context ctx = *this;
    ctx.is_embed = true;
    return ctx;
}

/// Validator

template <typename Node>
Validator<Node>::Validator(const Node &schema,
                           const std::map<std::string, TypeValidator> &type_validators)
    : m_settings{schema_settings(schema)}, m_types{schema_types(schema)},
      m_root{schema_root(schema)}, m_validators{type_validators} {

    // todo: add built-in generic types (list<T>, map<K;V>)
    static const std::map<std::string, TypeValidator> builtin_validators = {
        // basic
        {"any", [](const Node &) -> bool { return true; }},
        {"map", NodeAccessor::is_map},
        {"list", NodeAccessor::is_sequence},
        {"scalar", NodeAccessor::is_scalar},

        // numeric
        {"numeric", impl::node_is_number},
        {"num", impl::node_is_number},

        // integer
        {"integer", impl::node_is_integer},
        {"int", impl::node_is_integer},

        // bool
        {"boolean", impl::node_is_boolean},
        {"bool", impl::node_is_boolean},

        // string
        {"string", impl::node_is_string},
        {"str", impl::node_is_string},
    };

    m_validators.insert(builtin_validators.cbegin(), builtin_validators.cend());
}

template <typename Node>
auto Validator<Node>::validate(const Node &doc) const -> std::vector<Error> {
    const Context ctx{m_root};
    std::vector<Error> errors;
    validate(doc, m_root, ctx, errors);
    return errors;
}

template <typename Node>
auto Validator<Node>::schema_settings(const Node &schema) -> SchemaSettings {
    SchemaSettings settings;
    settings.default_required = true;
    settings.optional_tag = "optional";
    settings.required_tag = "required";
    settings.embed_tag = "embed";
    settings.variant_tag = "variant";
    settings.key_type_prefix = "$";
    settings.generic_brackets = "<>";
    settings.generic_separator = ";";
    settings.attribute_separator = ":";
    settings.ignore_attributes = false;

    const Node settings_node = NodeAccessor::at(schema, "settings");

    if (NodeAccessor::is_defined(settings_node)) {
        settings.default_required = NodeAccessor::as(
            NodeAccessor::at(settings_node, "default_required"), settings.default_required);
        settings.optional_tag = NodeAccessor::as(NodeAccessor::at(settings_node, "optional_tag"),
                                                 settings.optional_tag);
        settings.required_tag = NodeAccessor::as(NodeAccessor::at(settings_node, "required_tag"),
                                                 settings.required_tag);
        settings.embed_tag =
            NodeAccessor::as(NodeAccessor::at(settings_node, "embed_tag"), settings.embed_tag);
        settings.variant_tag =
            NodeAccessor::as(NodeAccessor::at(settings_node, "variant_tag"), settings.variant_tag);
        settings.key_type_prefix = NodeAccessor::as(
            NodeAccessor::at(settings_node, "key_type_prefix"), settings.key_type_prefix);
        settings.generic_brackets = NodeAccessor::as(
            NodeAccessor::at(settings_node, "generic_brackets"), settings.generic_brackets);
        settings.generic_separator = NodeAccessor::as(
            NodeAccessor::at(settings_node, "generic_separator"), settings.generic_separator);
        settings.attribute_separator = NodeAccessor::as(
            NodeAccessor::at(settings_node, "attribute_separator"), settings.attribute_separator);
        settings.ignore_attributes = NodeAccessor::as(
            NodeAccessor::at(settings_node, "ignore_attributes"), settings.ignore_attributes);
    }

    MIROIR_ASSERT(!settings.optional_tag.empty(), "optional tag name is empty");
    MIROIR_ASSERT(!settings.required_tag.empty(), "required tag name is empty");
    MIROIR_ASSERT(!settings.embed_tag.empty(), "embed tag name is empty");
    MIROIR_ASSERT(!settings.variant_tag.empty(), "variant tag name is empty");
    MIROIR_ASSERT(!settings.key_type_prefix.empty(), "key type prefix is empty");

    MIROIR_ASSERT(settings.generic_brackets.size() == 2,
                  "invalid generic brackets string length: " << settings.generic_brackets);
    MIROIR_ASSERT(settings.generic_separator.size() == 1,
                  "invalid generic separator string length: " << settings.generic_separator);

    MIROIR_ASSERT(settings.attribute_separator.size() == 1,
                  "invalid attribute separator string length: " << settings.attribute_separator);

    return settings;
}

template <typename Node>
auto Validator<Node>::schema_types(const Node &schema) -> std::map<std::string, Node> {
    return NodeAccessor::as(NodeAccessor::at(schema, "types"), std::map<std::string, Node>{});
}

template <typename Node> auto Validator<Node>::schema_root(const Node &schema) -> Node {
    MIROIR_ASSERT(NodeAccessor::is_map(schema),
                  "schema is not a map: " << NodeAccessor::dump(schema));
    const Node root = NodeAccessor::at(schema, "root");
    MIROIR_ASSERT(NodeAccessor::is_defined(root), "missing root node in the schema");
    return root;
}

template <typename Node>
auto Validator<Node>::make_error(ErrorType type, const Context &ctx,
                                 const std::vector<std::vector<Error>> &variant_errors) const
    -> Error {

    Error err;
    err.type = type;
    err.path = ctx.path;
    err.expected = ctx.expected;
    err.variant_errors = variant_errors;
    return err;
}

template <typename Node>
void Validator<Node>::validate(const Node &doc, const Node &schema, const Context &ctx,
                               std::vector<Error> &errors) const {

    if (NodeAccessor::is_scalar(schema)) {
        validate_scalar(doc, schema, ctx, errors);
    } else if (NodeAccessor::is_sequence(schema)) {
        validate_sequence(doc, schema, ctx, errors);
    } else if (NodeAccessor::is_map(schema)) {
        validate_map(doc, schema, ctx, errors);
    } else {
        MIROIR_ASSERT(false, "invalid schema node: " << NodeAccessor::dump(schema));
    }
}

template <typename Node>
void Validator<Node>::validate_type(const Node &doc, const std::string &type, const Context &ctx,
                                    std::vector<Error> &errors) const {

    // generic args
    // note: generic args can only contain names of other types, but not the types themselves, e.g.
    // "generic<string>" is a valid type, but "generic<[string]>" is not, define and use an alias
    // (e.g. "list<string>")
    if (!ctx.where.empty()) {
        const auto concrete_type_it = ctx.where.find(type);

        if (concrete_type_it != ctx.where.end()) {
            const std::string &concrete_type = concrete_type_it->second;
            validate_type(doc, concrete_type, ctx.with_expected(type).with_where({}), errors);
            return;
        }
    }

    // generic types
    if (type_is_generic(type)) {
        const GenericType generic_type = parse_generic_type(type);

        for (const auto &[schema_type, schema_type_node] : m_types) {
            if (!type_is_generic(schema_type)) {
                continue;
            }

            const GenericType generic_schema_type = parse_generic_type(schema_type);
            if (generic_type.name == generic_schema_type.name) {
                const std::map<std::string, std::string> generic_args =
                    make_generic_args(generic_schema_type, generic_type, ctx.where);
                validate(doc, schema_type_node, ctx.with_expected(type).with_where(generic_args),
                         errors);
                return;
            }
        }
    }

    // schema types
    const auto type_it = m_types.find(type);
    if (type_it != m_types.end()) {
        const Node schema_type_node = type_it->second;
        validate(doc, schema_type_node, ctx.with_expected(type).with_where({}), errors);
        return;
    }

    // built-in types
    const auto validator_it = m_validators.find(type);
    if (validator_it != m_validators.end()) {
        const TypeValidator type_validator = validator_it->second;

        if (!type_validator(doc)) {
            // node has invalid type
            const Error err = make_error(ErrorType::InvalidValueType, ctx.with_expected(type));
            errors.push_back(err);
        }

        return;
    }

    MIROIR_ASSERT(false, "type not found: " << type);
}

template <typename Node>
auto Validator<Node>::validate_type(const Node &doc, const std::string &type,
                                    const Context &ctx) const -> bool {

    std::vector<Error> errors;
    validate_type(doc, type, ctx.with_expected(type), errors);
    return errors.empty();
}

template <typename Node>
void Validator<Node>::validate_scalar(const Node &doc, const Node &schema, const Context &ctx,
                                      std::vector<Error> &errors) const {

    const std::string type = NodeAccessor::template as<std::string>(schema);
    validate_type(doc, type, ctx, errors);
}

template <typename Node>
void Validator<Node>::validate_sequence(const Node &doc, const Node &schema, const Context &ctx,
                                        std::vector<Error> &errors) const {

    const std::size_t schema_size = NodeAccessor::size(schema);

    if (schema_size == 0) {
        if (!NodeAccessor::is_sequence(doc)) {
            // schema node is an empty sequence but document node is not a sequence
            const Error err = make_error(ErrorType::InvalidValueType, ctx);
            errors.push_back(err);
        }

        // allow any sequence on empty sequence in the schema
        return;
    }

    const std::string schema_tag = NodeAccessor::tag(schema);

    if (tag_is_variant(schema_tag)) {
        for (auto it = NodeAccessor::begin(schema); it != NodeAccessor::end(schema); ++it) {
            if (NodeAccessor::equals(doc, *it)) {
                // found correct node value
                return;
            }
        }

        // document node has invalid value
        const Error err = make_error(ErrorType::InvalidValue, ctx);
        errors.push_back(err);
    } else if (schema_size == 1) {
        const Node child_schema_node = NodeAccessor::at(schema, 0);

        if (NodeAccessor::is_sequence(doc)) {
            for (std::size_t i = 0; i < NodeAccessor::size(doc); ++i) {
                const Node child_doc_node = NodeAccessor::at(doc, i);
                validate(child_doc_node, child_schema_node, ctx.appending_path(std::to_string(i)),
                         errors);
            }
        } else {
            // schema node is a sequence but document node is not a sequence
            const Error err = make_error(ErrorType::InvalidValueType, ctx);
            errors.push_back(err);
        }
    } else { // schema_size > 1
        std::vector<std::vector<Error>> grouped_errors;
        std::vector<Error> variant_errors;

        for (auto it = NodeAccessor::begin(schema); it != NodeAccessor::end(schema); ++it) {
            const Node variant_schema = *it;

            variant_errors.clear();
            validate(doc, variant_schema, ctx.with_expected(variant_schema), variant_errors);

            if (variant_errors.empty()) {
                // found correct node type
                return;
            }

            grouped_errors.push_back(variant_errors);
        }

        // document node has invalid type
        const Error err = make_error(ErrorType::InvalidValueType, ctx, grouped_errors);
        errors.push_back(err);
    }
}

template <typename Node>
void Validator<Node>::validate_map(const Node &doc, const Node &schema, const Context &ctx,
                                   std::vector<Error> &errors) const {

    const bool doc_is_map = NodeAccessor::is_map(doc);

    if (NodeAccessor::size(schema) == 0) {
        if (!doc_is_map) {
            // document node must be a map
            const Error err = make_error(ErrorType::InvalidValueType, ctx);
            errors.push_back(err);
        }

        // allow any map on empty map in the schema
        return;
    }

    std::vector<Node> validated_nodes;
    std::vector<std::pair<std::string, Node>> key_types; // key type, schema val node

    std::size_t embed_count = 0;
    bool has_required_nodes = false;

    // validate document structure
    for (auto it = NodeAccessor::begin(schema); it != NodeAccessor::end(schema); ++it) {
        const Node schema_val_node = it->second;
        const std::string schema_val_tag = NodeAccessor::tag(schema_val_node);

        if (tag_is_embed(schema_val_tag)) {
            if (doc_is_map) {
                validate(doc, schema_val_node, ctx.with_embed(), errors);
                ++embed_count;
            }
        } else {
            const Node schema_key_node = it->first;
            const std::string key = NodeAccessor::template as<std::string>(schema_key_node);

            if (!impl::string_is_prefixed(key, m_settings.key_type_prefix)) {
                const bool node_is_required = tag_is_required(schema_val_tag);
                const std::optional<Node> child_doc_node = find_node(doc, key);
                const Context child_ctx = ctx.appending_path(key);

                has_required_nodes = has_required_nodes || node_is_required;

                if (child_doc_node.has_value()) {
                    validate(child_doc_node.value(), schema_val_node, child_ctx, errors);
                    validated_nodes.push_back(child_doc_node.value());
                } else if (node_is_required) {
                    // required node not found
                    const Error err = make_error(ErrorType::NodeNotFound, child_ctx);
                    errors.push_back(err);
                }
            } else {
                const std::string key_type = key.substr(m_settings.key_type_prefix.size());
                key_types.emplace_back(key_type, schema_val_node);
            }
        }
    }

    if (!doc_is_map) {
        if (!has_required_nodes || !key_types.empty()) {
            // document node must be a map
            const Error err = make_error(ErrorType::InvalidValueType, ctx);
            errors.push_back(err);
        }

        return;
    }

    // validate key types
    for (const auto &[key_type, schema_val_node] : key_types) {
        const std::string schema_val_tag = NodeAccessor::tag(schema_val_node);
        bool key_type_is_valid = !tag_is_required(schema_val_tag);

        for (auto it = NodeAccessor::begin(doc); it != NodeAccessor::end(doc); ++it) {
            const Node child_doc_val_node = it->second;
            if (impl::nodes_contains_node(validated_nodes, child_doc_val_node)) {
                continue;
            }

            const Node child_doc_key_node = it->first;
            if (!validate_type(child_doc_key_node, key_type, ctx)) {
                continue;
            }

            key_type_is_valid = true;

            const std::string child_key =
                NodeAccessor::template as<std::string>(child_doc_key_node);
            validate(child_doc_val_node, schema_val_node, ctx.appending_path(child_key), errors);
            validated_nodes.push_back(child_doc_val_node);
        }

        if (!key_type_is_valid) {
            // didn't find a key with required type
            const Error err =
                make_error(ErrorType::MissingKeyWithType, ctx.with_expected(key_type));
            errors.push_back(err);
        }
    }

    // find undefined nodes
    for (auto it = NodeAccessor::begin(doc); it != NodeAccessor::end(doc); ++it) {
        const Node child_doc_val_node = it->second;
        if (impl::nodes_contains_node(validated_nodes, child_doc_val_node)) {
            continue;
        }

        const Node child_doc_key_node = it->first;
        const std::string child_key = NodeAccessor::template as<std::string>(child_doc_key_node);

        // node not defined in the schema
        const Error err = make_error(ErrorType::UndefinedNode, ctx.appending_path(child_key));
        errors.push_back(err);
    }

    // filter UndefinedNode errors
    if (!ctx.is_embed) {
        impl::filter_undefined_node_errors(errors, embed_count);
    }
}

template <typename Node>
auto Validator<Node>::tag_is_optional(const std::string &tag) const -> bool {
    return tag == m_settings.optional_tag;
}

template <typename Node> auto Validator<Node>::tag_is_embed(const std::string &tag) const -> bool {
    return tag == m_settings.embed_tag;
}

template <typename Node>
auto Validator<Node>::tag_is_variant(const std::string &tag) const -> bool {
    return tag == m_settings.variant_tag;
}

template <typename Node>
auto Validator<Node>::tag_is_required(const std::string &tag) const -> bool {
    return (m_settings.default_required && !tag_is_optional(tag)) ||
           (!m_settings.default_required && tag == m_settings.required_tag);
}

template <typename Node>
auto Validator<Node>::find_node(const Node &map, const std::string &key) const
    -> std::optional<Node> {

    if (!NodeAccessor::is_map(map)) {
        return std::nullopt;
    }

    const Node node = NodeAccessor::at(map, key);

    if (NodeAccessor::is_defined(node)) {
        return node;
    }

    if (m_settings.ignore_attributes) {
        for (auto it = NodeAccessor::begin(map); it != NodeAccessor::end(map); ++it) {
            const Node key_node = it->first;
            const Node val_node = it->second;
            const std::string node_key = NodeAccessor::template as<std::string>(key_node);

            if (impl::string_trim_after(node_key, m_settings.attribute_separator[0]) == key) {
                return std::optional<Node>{val_node};
            }
        }
    }

    return std::nullopt;
}

template <typename Node>
auto Validator<Node>::type_is_generic(const std::string &type) const -> bool {
    return m_generic_types_cache.count(type) > 0 ||
           type.find(m_settings.generic_brackets[0]) != std::string::npos;
}

template <typename Node>
auto Validator<Node>::parse_generic_type(const std::string &type) const -> GenericType {
    const auto cached_generic_type_it = m_generic_types_cache.find(type);
    if (cached_generic_type_it != m_generic_types_cache.end()) {
        const GenericType &cached_generic_type = cached_generic_type_it->second;
        return cached_generic_type;
    }

    GenericType generic_type{};

    enum {
        ST_NAME,
        ST_ARGS,
        ST_SEP,
        ST_END,
    } state = ST_NAME;

    int level = 0;
    std::string_view arg;

    for (auto it = type.cbegin(); it != type.cend(); ++it) {
        MIROIR_ASSERT(state != ST_END, "invalid generic parser intermediate state: " << type);

        const unsigned char c = *it;

        if (std::isspace(c)) {
            continue;
        }

        if (c == m_settings.generic_brackets[0]) {
            ++level;

            if (level == 1) {
                state = ST_ARGS;
                continue; // skip open bracket
            }
        } else if (c == m_settings.generic_brackets[1]) {
            --level;

            if (level == 0) {
                state = ST_END;
            }
        } else if (level == 1 && c == m_settings.generic_separator[0]) {
            state = ST_SEP;
        }

        switch (state) {
        case ST_NAME:
            generic_type.name += c;
            break;
        case ST_ARGS:
            arg = std::string_view{!arg.empty() ? arg.data() : &(*it), arg.size() + 1};
            break;
        case ST_SEP:
            state = ST_ARGS;
            [[fallthrough]];
        case ST_END:
            MIROIR_ASSERT(!arg.empty(), "generic arg is empty: " << type);
            generic_type.args.push_back(std::string{arg});
            arg = std::string_view{};
            break;
        default:
            MIROIR_ASSERT(false, "invalid generic parser state: " << state);
        }
    }

    MIROIR_ASSERT(level == 0, "generic brackets are disbalanced: " << type);
    MIROIR_ASSERT(state == ST_END, "invalid generic parser end state: " << type);
    MIROIR_ASSERT(!generic_type.name.empty(), "generic name is empty: " << type);
    MIROIR_ASSERT(!generic_type.args.empty(), "generic args are empty: " << type);

    m_generic_types_cache[type] = generic_type;

    // todo: return reference to cache?
    return generic_type;
}

template <typename Node>
auto Validator<Node>::make_generic_args(const GenericType &keys, const GenericType &vals,
                                        const std::map<std::string, std::string> &where) const
    -> std::map<std::string, std::string> {

    MIROIR_ASSERT(!keys.args.empty(), "generic args are empty");
    MIROIR_ASSERT(!vals.args.empty(), "generic args are empty");
    MIROIR_ASSERT(keys.args.size() == vals.args.size(), "generic args count mismatch");

    std::map<std::string, std::string> generic_args;

    for (std::size_t i = 0; i < keys.args.size(); ++i) {
        const std::string key = keys.args[i];
        const std::string val = vals.args[i];

        const auto it = where.find(val);
        if (it == where.end()) {
            generic_args[key] = val;
        } else {
            generic_args[key] = it->second;
        }
    }

    return generic_args;
}

} // namespace miroir

#endif // ifdef MIROIR_IMPLEMENTATION

#ifdef MIROIR_YAMLCPP_SPECIALIZATION

#include <yaml-cpp/yaml.h>

namespace miroir {

template <> struct NodeAccessor<YAML::Node> {
    using Node = YAML::Node;
    using Iterator = YAML::const_iterator;

    static auto is_defined(const Node &node) -> bool { return node.IsDefined(); }

    static auto is_explicit(const Node &node) -> bool {
        // see: https://yaml.org/spec/1.2.2/#24-tags
        // Explicit typing is denoted with a tag using the exclamation point (“!”) symbol.
        return node.Tag() == "!";
    }

    static auto is_scalar(const Node &node) -> bool { return node.IsScalar(); }
    static auto is_sequence(const Node &node) -> bool { return node.IsSequence(); }
    static auto is_map(const Node &node) -> bool { return node.IsMap(); }

    template <typename Key> static auto at(const Node &node, const Key &key) -> Node {
        MIROIR_ASSERT(node.IsSequence() || node.IsMap(),
                      "node is not a sequence or a map: " << dump(node));
        return node[key];
    }

    template <typename T> static auto as(const Node &node) -> T { return node.as<T>(); }

    template <typename T> static auto as(const Node &node, const T &fallback) -> T {
        return node.as<T, T>(fallback);
    }

    static auto tag(const Node &node) -> std::string { return node.Tag().substr(1); }

    static auto dump(const Node &node) -> std::string {
        YAML::Emitter emitter;
        emitter.SetSeqFormat(YAML::Flow);
        emitter.SetMapFormat(YAML::Flow);
        emitter << node;
        MIROIR_ASSERT(emitter.good(), "invalid node to emit");
        return emitter.c_str();
    }

    static auto equals(const Node &lhs, const Node &rhs) -> bool {
        return lhs == rhs || dump(lhs) == dump(rhs);
    }

    static auto is_same(const Node &lhs, const Node &rhs) -> bool { return lhs == rhs; }

    static auto size(const Node &node) -> std::size_t {
        MIROIR_ASSERT(node.IsSequence() || node.IsMap(),
                      "node is not a sequence or a map: " << dump(node));
        return node.size();
    }

    static auto begin(const Node &node) -> Iterator {
        MIROIR_ASSERT(node.IsSequence() || node.IsMap(),
                      "node is not a sequence or a map: " << dump(node));
        return node.begin();
    }

    static auto end(const Node &node) -> Iterator {
        MIROIR_ASSERT(node.IsSequence() || node.IsMap(),
                      "node is not a sequence or a map: " << dump(node));
        return node.end();
    }
};

} // namespace miroir

#endif // ifdef MIROIR_YAMLCPP_SPECIALIZATION
