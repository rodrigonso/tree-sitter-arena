#ifndef TREE_SITTER_TREE_H_
#define TREE_SITTER_TREE_H_

#include "./subtree.h"
#include "./alloc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const Subtree *child;
  const Subtree *parent;
  Length position;
  TSSymbol alias_symbol;
} ParentCacheEntry;

struct TSTree {
  Subtree root;
  const TSLanguage *language;
  TSRange *included_ranges;
  unsigned included_range_count;
  TSAllocator allocator;
};

TSTree *ts_tree_new(const TSAllocator *alloc, Subtree root, const TSLanguage *language, const TSRange *included_ranges, unsigned included_range_count);
TSNode ts_node_new(const TSTree *tree, const Subtree *subtree, Length position, TSSymbol alias);

#ifdef __cplusplus
}
#endif

#endif  // TREE_SITTER_TREE_H_
