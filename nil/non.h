#pragma once

#include "reflect.h"
#include "string.h"
#include "visit.h"

/*

A simple, human-readable serialization format called Nil Object Notation (NON) implemented using nil's reflection system.

Here's an example:

save_date "29 Nov 2025"
scene "rune:level/train"
// Line comment
entities [
	{
		name "Player"
		transform {
			translation [ 1, 2, 3 ]
			rotation []
			rotation [ 1, 1, 1 ]
		}
		scene_ref {
			id "rune:obj/player"
		}
		children [
			32
			11
			52
			3
		]
	}
]

Key-value pairs and entries are separated by spaces. All other whitespace is ignored.

*/

void non_write_reflected(FILE* file, const type_info* type, const void* data);

typedef enum non_node_kind : u32 { // To ensure padding in non_node
	non_map, // Children treated as a map.
	non_list, // Children treated as a simple list.
	non_string,
	non_char,
	non_number,
} non_node_kind;

constexpr u32 NON_VACANT = U32_MAX;

typedef usize non_file_location;

typedef struct non_node {
	non_node_kind kind;

	// u32 children_count;
	// usize children_start;

	// Nodes point to each other a little like a linked list.
	// If `next_sibling` or `first_child` are not `NON_VACANT`, they are indexes into the `nodes` array
	u32 next_sibling;
	u32 first_child;
	u32 child_count;

	non_file_location loc;

	union {
		str str;
		int chr;
		double num;
	};
} non_node;

typedef struct non_tree {
	// Array of nodes inside this tree. 0 is always the root node.
	non_node* nodes;
	usize node_count;
	char* contents;
} non_tree;

void non_tree_debug(const non_tree* tree);

/*typedef enum non_cursor_kind : u8 {
	non_string,
	non_char,
	non_number,
} non_cursor_kind;

typedef struct non_cursor {
	non_cursor_kind kind;
	union {
		str str;
		int chr;
		double num;
	};
} non_cursor;*/

/*typedef enum non_parse_error {
	non_parse_error_failed_to_read_file,
	non_parse_error_unclosed_paren,
	non_parse_error_incomplete_char,
} non_parse_error;*/


// TODO: Add file location data.
typedef struct non_result {
	bool ok;
	enum non_error {
		non_failed_to_read_file,
		non_unclosed_opener,
		non_unexpected_closer,
		non_incomplete_char,
		non_incomplete_pair,
		non_invalid_escape_character,
		non_invalid_map_key,
		non_invalid_map_value,
		non_invalid_enum_data, // Enum values either expect a string or variant value.
		non_invalid_number, // Unable to parse number.
		non_unable_to_read_opaque, // from_* either doesn't exist or failed.
		non_target_type_not_list, // Tried to read a list but the type being read into isn't a list.
		non_target_type_not_defaulted, // Tried to read a type that requires creating a default value, but the trait was not implemented.
		non_unable_to_allocate_field, // Loose pointer with no allocation instructions, unable to allocate field.
	} error;
	non_file_location loc;
} non_result;
constexpr non_result non_ok = { .ok = true };
static inline non_result non_error(const enum non_error error, const non_file_location loc) {
	return (non_result){ .ok = false, .error = error, .loc = loc };
}

void non_print_error(FILE* file, non_result result, str source);

// typedef void (*non_error_visitor)(non_parse_error err, void* client_data);
// typedef nil_tree_visitor_step (*non_node_visitor)(non_cursor cursor, void* client_data);

non_result non_parse(string s, non_tree* tree);
void non_free(non_tree tree);
// void non_parse(FILE* file, non_node_visitor visitor, void* client_data, non_error_visitor error_visitor, void* error_visitor_data);

non_result non_read_into_reflected(FILE* file, const type_info* type, void* data);