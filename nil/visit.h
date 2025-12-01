#pragma once
// Utilities for visitor APIs

// How to move forward after a visitor function has run on a tree structure.
typedef enum nil_tree_visitor_step {
	// Stop visiting and exit.
	nil_break,
	// Skip recursing children of this node, and go to its next sibling instead.
	nil_continue,
	// Recurse down the tree.
	nil_recurse,
} nil_tree_visitor_step;