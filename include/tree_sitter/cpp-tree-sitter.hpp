#ifndef CPP_TREE_SITTER_H
#define CPP_TREE_SITTER_H

#include <memory>
#include <string_view>

#include <tree_sitter/api.h>
#include <tree_sitter/parser.h>

#include "tree_sitter/langs.hpp"

// Including the API directly already pollutes the namespace, but the
// functions are prefixed. Anything else that we include should be scoped
// within a namespace.

namespace ts
{

    /////////////////////////////////////////////////////////////////////////////
    // Helper classes.
    // These can be ignored while tring to understand the core APIs on demand.
    /////////////////////////////////////////////////////////////////////////////

    struct free_helper
    {
        template <typename T>
        auto operator()(T *raw_pointer) const -> void
        {
            std::free(raw_pointer);
        }
    };

    // An inclusive range representation
    template <typename T>
    struct extent
    {
        T start;
        T end;
    };

    /////////////////////////////////////////////////////////////////////////////
    // Aliases.
    // Create slightly stricter aliases for some of the core tree-sitter types.
    /////////////////////////////////////////////////////////////////////////////

    // Direct alias of { row: uint32_t; column: uint32_t }
    using point = TSPoint;

    using symbol = uint16_t;

    using version = uint32_t;

    using node_id = uintptr_t;

    // For types that manage resources, create custom wrappers that ensure
    // clean-up. For types that can benefit from additional API discovery,
    // wrappers with implicit conversion allow for automated method discovery.

    struct language
    {
        // NOTE: Allowing implicit conversions from TSLanguage to Language
        // improves the ergonomics a bit, as clients will still make use of the
        // custom language functions.

        /* implicit */
        language(TSLanguage const *language)
            : impl{language}
        {
        }

        [[nodiscard]] auto get_num_symbols() const -> size_t
        {
            return ts_language_symbol_count(impl);
        }

        [[nodiscard]] auto get_symbol_name(symbol symbol) const -> std::string_view
        {
            return ts_language_symbol_name(impl, symbol);
        }

        [[nodiscard]] auto get_symbol_for_name(std::string_view name, bool isNamed) const -> symbol
        {
            return ts_language_symbol_for_name(impl,
                                               &name.front(),
                                               static_cast<uint32_t>(name.size()),
                                               isNamed);
        }

        [[nodiscard]] auto get_version() const -> version
        {
            return ts_language_version(impl);
        }

        TSLanguage const *impl;
    };

    class cursor;

    struct node
    {
        explicit node(TSNode node)
            : impl{node}
        {
        }

        ////////////////////////////////////////////////////////////////
        // Flag checks on nodes
        ////////////////////////////////////////////////////////////////
        [[nodiscard]] auto is_null() const -> bool
        {
            return ts_node_is_null(impl);
        }

        [[nodiscard]] auto is_named() const -> bool
        {
            return ts_node_is_named(impl);
        }

        [[nodiscard]] auto is_missing() const -> bool
        {
            return ts_node_is_missing(impl);
        }

        [[nodiscard]] auto is_extra() const -> bool
        {
            return ts_node_is_extra(impl);
        }

        [[nodiscard]] auto has_error() const -> bool
        {
            return ts_node_has_error(impl);
        }

        // TODO: Not yet available in last release
        // [[nodiscard]] bool
        // isError() const {
        //   return ts_node_is_error(impl);
        // }

        ////////////////////////////////////////////////////////////////
        // Navigation
        ////////////////////////////////////////////////////////////////

        // Direct parent/sibling/child connections and cursors

        [[nodiscard]] auto get_parent() const -> node
        {
            return node{ts_node_parent(impl)};
        }

        [[nodiscard]] auto get_previous_sibling() const -> node
        {
            return node{ts_node_prev_sibling(impl)};
        }

        [[nodiscard]] auto get_next_sibling() const -> node
        {
            return node{ts_node_next_sibling(impl)};
        }

        [[nodiscard]] auto get_num_children() const -> uint32_t
        {
            return ts_node_child_count(impl);
        }

        [[nodiscard]] auto get_child(uint32_t position) const -> node
        {
            return node{ts_node_child(impl, position)};
        }

        // Named children

        [[nodiscard]] auto get_num_named_children() const -> uint32_t
        {
            return ts_node_named_child_count(impl);
        }

        [[nodiscard]] auto get_named_child(uint32_t position) const -> node
        {
            return node{ts_node_named_child(impl, position)};
        }

        // Named fields

        [[nodiscard]] auto get_field_name_for_child(uint32_t child_position) const -> std::string_view
        {
            return ts_node_field_name_for_child(impl, child_position);
        }

        [[nodiscard]] auto get_child_by_field_name(std::string_view name) const -> node
        {
            return node{ts_node_child_by_field_name(impl,
                                                    &name.front(),
                                                    static_cast<uint32_t>(name.size()))};
        }

        // Definition deferred until after the definition of Cursor.
        [[nodiscard]] auto get_cursor() const -> cursor;

        ////////////////////////////////////////////////////////////////
        // Node attributes
        ////////////////////////////////////////////////////////////////

        // Returns a unique identifier for a node in a parse tree.
        // NodeIDs are numeric value types.
        [[nodiscard]] auto get_id() const -> node_id
        {
            return reinterpret_cast<node_id>(impl.id);
        }

        // Returns an S-Expression representation of the subtree rooted at this node.
        [[nodiscard]] auto get_string_expr() const -> std::unique_ptr<char, free_helper>
        {
            return std::unique_ptr<char, free_helper>{ts_node_string(impl)};
        }

        [[nodiscard]] auto get_symbol() const -> symbol
        {
            return ts_node_symbol(impl);
        }

        [[nodiscard]] auto get_type() const -> std::string_view
        {
            return ts_node_type(impl);
        }

        // TODO: Not yet available in last release
        // [[nodiscard]] Language
        // getLanguage() const {
        //   return ts_node_language(impl);
        // }

        [[nodiscard]] auto get_byte_range() const -> extent<uint32_t>
        {
            return {ts_node_start_byte(impl), ts_node_end_byte(impl)};
        }

        [[nodiscard]] auto get_point_range() const -> extent<point>
        {
            return {ts_node_start_point(impl), ts_node_end_point(impl)};
        }

        [[nodiscard]] auto get_source_range(std::string_view source) const -> std::string_view
        {
            extent<uint32_t> extents = this->get_byte_range();
            return source.substr(extents.start, extents.end - extents.start);
        }

        TSNode impl;
    };

    class tree
    {
    public:
        tree(TSTree *tree)
            : impl{tree, ts_tree_delete}
        {
        }

        [[nodiscard]] auto get_root_node() const -> node
        {
            return node{ts_tree_root_node(impl.get())};
        }

        [[nodiscard]] auto get_language() const -> language
        {
            return language{ts_tree_language(impl.get())};
        }

        [[nodiscard]] auto has_error() const -> bool
        {
            return get_root_node().has_error();
        }

    private:
        std::unique_ptr<TSTree, decltype(&ts_tree_delete)> impl;
    };

    class parser
    {
    public:
        parser(language language)
            : impl{ts_parser_new(), ts_parser_delete}
        {
            ts_parser_set_language(impl.get(), language.impl);
        }

        [[nodiscard]] auto parse_string(std::string_view buffer) -> tree
        {
            return ts_parser_parse_string(
                impl.get(),
                nullptr,
                &buffer.front(),
                static_cast<uint32_t>(buffer.size()));
        }

    private:
        std::unique_ptr<TSParser, decltype(&ts_parser_delete)> impl;
    };

    class cursor
    {
    public:
        cursor(TSNode node)
            : impl{ts_tree_cursor_new(node)}
        {
        }

        cursor(const TSTreeCursor &cursor)
            : impl{ts_tree_cursor_copy(&cursor)}
        {
        }

        // By default avoid copies and moves until the ergonomics are clearer.
        cursor(const cursor &) = delete;
        cursor(cursor &&) = delete;
        cursor &operator=(const cursor &other) = delete;
        cursor &operator=(cursor &&other) = delete;

        ~cursor()
        {
            ts_tree_cursor_delete(&impl);
        }

        auto reset(node node) -> void
        {
            ts_tree_cursor_reset(&impl, node.impl);
        }

        // TODO: Not yet available in last release
        // void
        // reset(Cursor& cursor) {
        //   ts_tree_cursor_reset_to(&impl, &cursor.impl);
        // }

        [[nodiscard]] auto copy() const -> cursor
        {
            return cursor(impl);
        }

        [[nodiscard]] auto get_current_node() const -> node
        {
            return node{ts_tree_cursor_current_node(&impl)};
        }

        // Navigation

        [[nodiscard]] auto goto_parent() -> bool
        {
            return ts_tree_cursor_goto_parent(&impl);
        }

        [[nodiscard]] auto goto_next_sibling() -> bool
        {
            return ts_tree_cursor_goto_next_sibling(&impl);
        }

        // TODO: Not yet available in last release
        // [[nodiscard]] bool
        // gotoPreviousSibling() {
        //   return ts_tree_cursor_goto_previous_sibling(&impl);
        // }

        [[nodiscard]] auto goto_first_child() -> bool
        {
            return ts_tree_cursor_goto_first_child(&impl);
        }

        // TODO: Not yet available in last release
        // [[nodiscard]] bool
        // gotoLastChild() {
        //   return ts_tree_cursor_goto_last_child(&impl);
        // }

        // TODO: Not yet available in last release
        // [[nodiscard]] size_t
        // getDepthFromOrigin() const {
        //   return ts_tree_cursor_current_depth(&impl);
        // }

    private:
        TSTreeCursor impl;
    };

    // To avoid cyclic dependencies and ODR violations, we define all methods
    // *using* Cursors inline after the definition of Cursor itself.
    [[nodiscard]] auto inline node::get_cursor() const -> cursor
    {
        return cursor{impl};
    }

}

#endif