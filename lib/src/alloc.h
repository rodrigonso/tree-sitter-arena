#ifndef TREE_SITTER_ALLOC_H_
#define TREE_SITTER_ALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree_sitter/api.h"

#if defined(TREE_SITTER_HIDDEN_SYMBOLS) || defined(_WIN32)
#define TS_PUBLIC
#else
#define TS_PUBLIC __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Per-instance allocator macros — used by all internal code.
// These take a TSAllocator* as the first argument.
// ---------------------------------------------------------------------------
#define ts_alloc_malloc(alloc, size)        ((alloc)->malloc((size), (alloc)->ctx))
#define ts_alloc_calloc(alloc, count, size) ((alloc)->calloc((count), (size), (alloc)->ctx))
#define ts_alloc_free(alloc, ptr)           ((alloc)->free((ptr), (alloc)->ctx))

// ---------------------------------------------------------------------------
// Global default allocator — used by backward-compatible APIs
// (ts_parser_new, ts_query_new, etc.) and external scanners.
// ---------------------------------------------------------------------------
TS_PUBLIC extern TSAllocator ts_builtin_allocator;

// Legacy global function pointers — kept for TREE_SITTER_REUSE_ALLOCATOR
// compat with external scanners. These are wrappers around ts_builtin_allocator.
TS_PUBLIC extern void *(*ts_current_malloc)(size_t size);
TS_PUBLIC extern void *(*ts_current_calloc)(size_t count, size_t size);
TS_PUBLIC extern void (*ts_current_free)(void *ptr);

// Legacy macros for external scanners and backward compat code.
// New internal code should use ts_alloc_malloc/calloc/free instead.
#ifndef ts_malloc
#define ts_malloc  ts_current_malloc
#endif
#ifndef ts_calloc
#define ts_calloc  ts_current_calloc
#endif
#ifndef ts_free
#define ts_free    ts_current_free
#endif

#ifdef __cplusplus
}
#endif

#endif // TREE_SITTER_ALLOC_H_

