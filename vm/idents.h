#ifndef IDENTS_DEF
#define IDENTS_DEF

#include "value.h"
#include <stdlib.h>

typedef struct TreeNode {
	V ident;
	struct TreeNode *left;
	struct TreeNode *right;
} ITreeNode;

V lookup_ident(size_t, const char*);
int ident_count();

#endif
