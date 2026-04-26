#include "alloc.h"
#include "tree_sitter/api.h"
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Default allocator implementation (wraps libc, aborts on failure)
// ---------------------------------------------------------------------------

static void *ts_libc_malloc(size_t size, void *ctx) {
  (void)ctx;
  void *result = malloc(size);
  if (size > 0 && !result) {
    fprintf(stderr, "tree-sitter failed to allocate %zu bytes", size);
    abort();
  }
  return result;
}

static void *ts_libc_calloc(size_t count, size_t size, void *ctx) {
  (void)ctx;
  void *result = calloc(count, size);
  if (count > 0 && !result) {
    fprintf(stderr, "tree-sitter failed to allocate %zu bytes", count * size);
    abort();
  }
  return result;
}

static void ts_libc_free(void *ptr, void *ctx) {
  (void)ctx;
  free(ptr);
}

// The global default allocator used by ts_parser_new() and friends.
TS_PUBLIC TSAllocator ts_builtin_allocator = {
  ts_libc_malloc,
  ts_libc_calloc,
  ts_libc_free,
  NULL
};

// ---------------------------------------------------------------------------
// Legacy global function pointers for TREE_SITTER_REUSE_ALLOCATOR scanners.
// These delegate to ts_builtin_allocator.
// ---------------------------------------------------------------------------

static void *ts_compat_malloc(size_t size) {
  return ts_builtin_allocator.malloc(size, ts_builtin_allocator.ctx);
}

static void *ts_compat_calloc(size_t count, size_t size) {
  return ts_builtin_allocator.calloc(count, size, ts_builtin_allocator.ctx);
}

static void ts_compat_free(void *ptr) {
  ts_builtin_allocator.free(ptr, ts_builtin_allocator.ctx);
}

// Legacy realloc stub for external scanners that may still call ts_realloc.
// The core library no longer uses realloc, but scanner code compiled with
// TREE_SITTER_REUSE_ALLOCATOR will resolve ts_current_realloc at link time.
static void *ts_compat_realloc(void *ptr, size_t size) {
  return realloc(ptr, size);
}

TS_PUBLIC void *(*ts_current_malloc)(size_t) = ts_compat_malloc;
TS_PUBLIC void *(*ts_current_calloc)(size_t, size_t) = ts_compat_calloc;
TS_PUBLIC void *(*ts_current_realloc)(void *, size_t) = ts_compat_realloc;
TS_PUBLIC void (*ts_current_free)(void *) = ts_compat_free;

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

const TSAllocator *ts_default_allocator(void) {
  return &ts_builtin_allocator;
}

void ts_set_allocator(
  void *(*new_malloc)(size_t size),
  void *(*new_calloc)(size_t count, size_t size),
  void *(*new_realloc)(void *ptr, size_t size),
  void (*new_free)(void *ptr)
) {
  (void)new_realloc; // realloc is no longer used internally

  // Update legacy function pointers (for external scanners).
  ts_current_malloc = new_malloc ? new_malloc : ts_compat_malloc;
  ts_current_calloc = new_calloc ? new_calloc : ts_compat_calloc;
  ts_current_free = new_free ? new_free : ts_compat_free;
}

