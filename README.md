# tree-sitter-arena

> A fork of [tree-sitter](https://github.com/tree-sitter/tree-sitter) with **per-instance custom allocator support**.

[![Upstream](https://img.shields.io/badge/upstream-tree--sitter%2Ftree--sitter-blue)](https://github.com/tree-sitter/tree-sitter)

Tree-sitter is a parser generator tool and an incremental parsing library. It can build a concrete syntax tree for a source file and efficiently update the syntax tree as the source file is edited. Tree-sitter aims to be:

- **General** enough to parse any programming language
- **Fast** enough to parse on every keystroke in a text editor
- **Robust** enough to provide useful results even in the presence of syntax errors
- **Dependency-free** so that the runtime library (which is written in pure C) can be embedded in any application

## What This Fork Adds

This fork introduces a `TSAllocator` struct that lets you provide your own `malloc`, `calloc`, and `free` implementations **per parser instance** — with a `void* ctx` user-data pointer for stateful allocators like arenas and pools.

### Key changes from upstream

- **New `TSAllocator` type** with `malloc`, `calloc`, `free`, and `void* ctx`
- **No `realloc` in the interface** — all internal resize operations use `malloc` + `memcpy` (old size is always known at each call site), making arena/bump allocators trivial to implement
- **Per-instance allocators** — each `TSParser`, `TSTree`, `TSQuery`, and `TSQueryCursor` can use its own allocator
- **Raised internal pool caps** — subtree and stack node pools are larger (65536), reducing overflow to `free` and improving arena compatibility
- **Full backward compatibility** — `ts_parser_new()`, `ts_set_allocator()`, and all existing APIs still work unchanged

### New API

```c
// Custom allocator — no realloc needed
typedef struct TSAllocator {
    void *(*malloc)(size_t size, void *ctx);
    void *(*calloc)(size_t count, size_t size, void *ctx);
    void (*free)(void *ptr, void *ctx);
    void *ctx;
} TSAllocator;

// Create objects with a custom allocator
TSParser *ts_parser_new_with_allocator(const TSAllocator *allocator);
TSQuery *ts_query_new_with_allocator(const TSAllocator *allocator, ...);
TSQueryCursor *ts_query_cursor_new_with_allocator(const TSAllocator *allocator);

// Get the default (libc) allocator
const TSAllocator *ts_default_allocator(void);
```

### Example: Arena Allocator

```c
void *arena_malloc(size_t size, void *ctx) {
    Arena *arena = (Arena *)ctx;
    return arena_alloc(arena, size, 16);
}

void *arena_calloc(size_t count, size_t size, void *ctx) {
    Arena *arena = (Arena *)ctx;
    void *p = arena_alloc(arena, count * size, 16);
    memset(p, 0, count * size);
    return p;
}

void arena_free(void *ptr, void *ctx) {
    (void)ptr; (void)ctx; // No-op — arena frees in bulk
}

// Usage
Arena arena = create_arena();
TSAllocator alloc = { arena_malloc, arena_calloc, arena_free, &arena };

TSParser *parser = ts_parser_new_with_allocator(&alloc);
ts_parser_set_language(parser, language);
TSTree *tree = ts_parser_parse_string(parser, NULL, source, length);

// ... use tree ...

ts_tree_delete(tree);     // no-op frees (arena)
ts_parser_delete(parser); // no-op frees (arena)
arena_release(&arena);    // bulk free everything
```

### Design Notes

- The `TSAllocator` struct is **copied** into the parser — callers don't need to keep it alive (but `ctx` must remain valid)
- `TSTree` **inherits** the allocator from its parser at parse time
- External scanners still use the global legacy allocator (`ts_set_allocator`) — they are unaffected by per-instance allocators

### Arena Allocator Coverage

All core library code paths use the per-instance `TSAllocator`:

| Component | Arena Support |
|-----------|:------------:|
| `TSParser` (parser.c, stack.c, lexer.c) | ✅ |
| `TSTree` (tree.c, subtree.c, tree_cursor.c) | ✅ |
| `TSQuery` / `TSQueryCursor` (query.c) | ✅ |
| Array/pool internals (array.h, reusable_node.h) | ✅ |
| Changed ranges (get_changed_ranges.c) | ✅ |
| Wasm store (wasm_store.c) | ❌ Uses global legacy allocator |
| External scanners | ❌ Uses global legacy allocator (`ts_set_allocator`) |

> **Note:** The Wasm store (`wasm_store.c`) is behind the optional `wasm` feature flag and is not compiled by default. It still uses the legacy global allocator (`ts_malloc`/`ts_free`). Wasm language support is unaffected by per-instance allocators.

## Links

- [Upstream tree-sitter](https://github.com/tree-sitter/tree-sitter)
- [Documentation](https://tree-sitter.github.io)
- [Rust binding](lib/binding_rust/README.md)
- [Wasm binding](lib/binding_web/README.md)
- [Command-line interface](crates/cli/README.md)
