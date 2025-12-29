#include "non.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "panic.h"
#include "trait.h"
#include "vec.h"

static void indent(FILE* file, const u32 depth) {
	for (u32 i = 0; i < depth; i++)
		fputc('\t', file);
}

static void non_write_reflected_recursive(FILE* file, const type_info* type, const void* data, const u32 depth) {
	const auto list = trait_get(list_trait, type);
	if (list && list->const_iter) {
		fputs("[\n", file);
		dynamic_iterator iter = list->const_iter(list, data);
		void* element;

		while (iter.next(&iter, &element)) {
			indent(file, depth + 1);
			non_write_reflected_recursive(file, list->element_type, element, depth + 1);
			fputc('\n', file);
		}

		iter.free(&iter);

		indent(file, depth);
		fputc(']', file);
		return;
	}

	if (type->kind == type_info_struct) {
		u32 inner_depth = depth;
		// The root struct is implied.
		if (depth != 0) {
			fputs("{\n", file);
			inner_depth++;
		}

		for (usize i = 0; i < type->struct_data.field_count; i++) {
			const type_info_field* field = &type->struct_data.fields[i];
			indent(file, inner_depth);
			fputs(field->name.data, file);
			fputc(' ', file);
			non_write_reflected_recursive(file, field->type, type_info_resolve_field_const_ptr(field, data), inner_depth);
			fputc('\n', file);
		}

		if (depth != 0) {
			indent(file, depth);
			fputs("}\n", file);
		}
	} else if (type->kind == type_info_enum) {
		const s64 value = nil_bytes_to_integer(data, type->size);
		fputs(reflect_enum_name_from_variant_value(type, value).data, file);
	} else if (type->kind == type_info_union) {
		panic("TODO: Unimplemented. I need to add tagged union support!");
	} else if (type->kind == type_info_opaque) {
		const auto conversions = trait_get(primitive_conversion_trait, type);
		if (conversions == nullptr || conversions->write_string == nullptr)
			panic("Opaque type \"%s\" doesn't have the `write_string` primitive conversion registered!", type->name.data);

		if (type->opaque_data.kind == type_info_opaque_string) {
			fputc('"', file);
			conversions->write_string(data, file);
			fputc('"', file);
		} else {
			conversions->write_string(data, file);
		}
	}
}

void non_write_reflected(FILE* file, const type_info* type, const void* data) {
	non_write_reflected_recursive(file, type, data, 0);
}

// static void non_parse_into_node()

typedef struct token {
	enum token_type {
		token_brace_left,
		token_brace_right,
		token_bracket_left,
		token_bracket_right,
		token_string,
		token_char,
		token_number,
	} type;

	u32 node_binding;
	non_file_location loc;

	union {
		str str;
		int chr;
		double num;
	};
} token;

/*static const char* token_type_string(const enum token_type type) {
	switch (type) {
		case token_brace_left: return "token_brace_left";
		case token_brace_right: return "token_brace_right";
		case token_bracket_left: return "token_bracket_left";
		case token_bracket_right: return "token_bracket_right";
		case token_string: return "token_string";
		case token_char: return "token_char";
		case token_number: return "token_number";
	}
	return nullptr;
}*/

#define non_try(EXPR) do { const non_result non_try_res = (EXPR); if (!non_try_res.ok) return non_try_res; } while (false)

// Called to finish raw (unquoted) tokens.
static non_result finish_token(vec(token)* tokens, const char* token_start, const char* cursor, non_file_location loc) {
	if (cursor == token_start)
		return non_ok;

	const str s = { .data = token_start, .len = cursor - token_start };

	if (s.len < 128 && isdigit(*token_start)) {
		// Needs to be null-terminated for parsing.
		char buf[128];
		memcpy(buf, s.data, s.len);
		buf[s.len] = '\0';

		char* end;
		const double value = strtod(buf, &end);
		if (end == buf)
			return non_error(non_invalid_number, loc);
		vec_push(tokens, (token){ .type = token_number, .loc = loc, .num = value });
	} else {
		vec_push(tokens, (token){ .type = token_string, .loc = loc, .str = s });
	}

	return non_ok;
}

static non_result non_tokenize(const string s, vec(token)* tokens) {
	const char* token_start = s.data;
	const char* end = s.data + s.len + 1;

	char prev_chr = 0; // Used for 2 character long patterns like the start of comments.
	for (const char* cursor = s.data; cursor < end; prev_chr = *cursor, cursor++) {
		const non_file_location loc = token_start - s.data;

		// Eat whitespace and finish tokens.
		while (cursor < end && isspace(*cursor)) {
			non_try(finish_token(tokens, token_start, cursor, loc));

			cursor++;
			token_start = cursor;
		}

		// Line comments.
		if (prev_chr == '/' && *cursor == '/') {
			non_try(finish_token(tokens, token_start, cursor-1, loc));
			// Skip to next newline.
			while (++cursor < end && *cursor != '\n') {}
			token_start = cursor + 1;
			continue;
		}

		// Block comments.
		if (prev_chr == '/' && *cursor == '*') {
			non_try(finish_token(tokens, token_start, cursor-1, loc));
			cursor++; // Remove /*/ being a valid block comment.
			while (++cursor < end && *cursor != '/' && cursor[-1] != '*') {}
			token_start = cursor + 1;
			continue;
		}

		// Strings.
		if (*cursor == '"') {
			non_try(finish_token(tokens, token_start, cursor, loc));
			token_start = cursor + 1;
			while (++cursor < end && *cursor != '"') {}
			vec_push(tokens, (token){ .type = token_string, .loc = loc, .str = (str){ .data = token_start, .len = cursor - token_start } });
			// cursor++; // Skip trailing quote
			token_start = cursor+1;
			continue;
		}

		// Characters.
		if (*cursor == '\'') {
			non_try(finish_token(tokens, token_start, cursor, loc));
			// TODO: Better character handling.
			if (cursor + 2 >= end) {
				// error_visitor(non_parse_error_incomplete_char, error_visitor_data);
				return non_error(non_incomplete_char, loc);
			}

			int chr;

			if (cursor[1] == '\\') {
				if (cursor + 3 >= end || cursor[3] != '\'')
					return non_error(non_incomplete_char, loc);

				switch (cursor[2]) {
					case '\\':
						chr = '\\';
						break;
					case 'n':
						chr = '\n';
						break;
					case 't':
						chr = '\t';
						break;
					default: return non_error(non_invalid_escape_character, loc);
				}

				cursor += 3;
			} else {
				if (cursor[2] != '\'')
					return non_error(non_incomplete_char, loc);

				chr = (int)cursor[1];
				cursor += 2;
			}

			vec_push(tokens, (token){ .type = token_char, .loc = loc, .chr = chr });

			token_start = cursor;
			continue;
		}

		// Numbers.
		/*if (cursor == token_start && isdigit(*cursor)) {
			// double num;
			// sscanf();
		}*/

		// Maps.
		if (*cursor == '{') {
			non_try(finish_token(tokens, token_start, cursor, loc));
			vec_push(tokens, (token){ .type = token_brace_left, .loc = loc });
			token_start = cursor + 1;
			continue;
		}
		if (*cursor == '}') {
			non_try(finish_token(tokens, token_start, cursor, loc));
			vec_push(tokens, (token){ .type = token_brace_right, .loc = loc });
			token_start = cursor + 1;
			continue;
		}

		// Arrays.
		if (*cursor == '[') {
			non_try(finish_token(tokens, token_start, cursor, loc));
			vec_push(tokens, (token){ .type = token_bracket_left, .loc = loc });
			token_start = cursor + 1;
			continue;
		}
		if (*cursor == ']') {
			non_try(finish_token(tokens, token_start, cursor, loc));
			vec_push(tokens, (token){ .type = token_bracket_right, .loc = loc });
			token_start = cursor + 1;
			continue;
		}
	}

	return non_ok;
}

/*typedef enum parse_scope_type {
	scope_map,
	scope_list,
} parse_scope_type;*/

typedef struct parse_ctx {
	non_node* nodes;
	u32 node_count;
	u32 nodes_head;
} parse_ctx;

/*static token* skip_scope(token* token_start, const token* token_end) {
	usize depth = 0;
	for (token* token = token_start; token < token_end; token++) {
		switch (token->type) {
			case token_brace_left: case token_bracket_left:
				depth++;
				break;
			case token_brace_right: case token_bracket_right:
				depth--;
				break;
			default:
		}

		if (depth == 0)
			return token;
	}
	return token_start;
}

static non_result parse_scope(token* token_start, token* token_end, non_node* parent, parse_ctx* ctx) {
	parent->children_start = ctx->nodes_head;

	// False if we are parsing a key, true if we are parsing a value.
	bool value_state = false;

#define PUSH_NODE(...) \
	ctx->nodes[ctx->nodes_head] = (non_node){ __VA_ARGS__ }; \
	token->node_binding = ctx->nodes_head; \
	ctx->nodes_head++; \
	parent->children_count++;

	// First pass: ignore children, allocate sibling nodes.
	for (token* token = token_start; token < token_end; token++) {
		switch (token->type) {
			case token_brace_left:
				if (parent->kind == non_map) {
					token = skip_scope(token, token_end);
				} else {
					PUSH_NODE( .kind = non_map );
				}
				break;

			case token_bracket_left:
				if (parent->kind == non_map) {
					token = skip_scope(token, token_end);
				} else {
					PUSH_NODE( .kind = non_list );
				}
				break;

			// If this is the end of this scope, let's jump to the second pass.
			case token_brace_right: case token_bracket_right:
				goto build_children;

			// TODO: remove this code duplication
			case token_string:
				PUSH_NODE( .kind = non_string, .str = token->str );
				break;
			case token_char:
				PUSH_NODE( .kind = non_char, .chr = token->chr );
				break;
			case token_number:
				PUSH_NODE( .kind = non_number, .num = token->num );
				break;
		}
	}

	// Second pass: ignore sibling nodes, allocate children recursively.
	build_children:
	for (token* token = token_start; token < token_end; token++) {
		switch (token->type) {
			case token_brace_left: case token_bracket_left:
				if (parent->kind == non_map) {
					const struct token* key_token = &token[-1];
					non_node* key_node = &ctx->nodes[key_token->node_binding];

					ctx->nodes[ctx->nodes_head] = (non_node){ .kind = token->type == token_brace_left ? non_map : non_list };
					token->node_binding = ctx->nodes_head;
					key_node->children_start = ctx->nodes_head;
					key_node->children_count++;
					ctx->nodes_head++;
				}

				parse_scope(token+1, token_end, &ctx->nodes[token->node_binding], ctx);

				break;

			case token_brace_right: case token_bracket_right:
				return non_ok;

			default:
		}
	}
#undef PUSH_NODE

	UNREACHABLE;
}*/

static const char* error_code_string(const enum non_error error) {
	switch (error) {
		case non_failed_to_read_file: return "Failed to read file";
		case non_unclosed_opener: return "Unclosed { or [";
		case non_unexpected_closer: return "Unexpected closer";
		case non_incomplete_char: return "Incomplete char";
		case non_incomplete_pair: return "Incomplete pair";
		case non_invalid_escape_character: return "Invalid escape character";
		case non_invalid_map_key: return "Invalid map key";
		case non_invalid_map_value: return "Invalid map value";
		case non_invalid_enum_data: return "Invalid enum data";
		case non_invalid_number: return "Invalid number";
		case non_unable_to_read_opaque: return "Unable to read opaque type";
		case non_target_type_not_list: return "Tried to read a list but the type being read into isn't a list";
		case non_target_type_not_defaulted: return "Tried to read a type that requires creating a default value, but the trait was not implemented";
		case non_unable_to_allocate_field: return "Loose pointer with no allocation instructions, unable to allocate field";
	}
	UNREACHABLE;
}

static void non_tree_debug_internal(const non_tree* tree, const non_node* node, const u32 indent) {
	for (int i = 0; i < indent; i++)
		putc('\t', stdout);

	switch (node->kind) {
		case non_map:
			printf("map\n");
			break;
		case non_list:
			printf("list\n");
			break;
		case non_string:
			putc('"', stdout);
			str_write(node->str, stdout);
			printf("\"\n");
			break;
		case non_char:
			putc('\'', stdout);
			putc(node->chr, stdout);
			printf("'\n");
			break;
		case non_number:
			printf("%lf\n", node->num);
			break;
	}

	if (node->first_child != NON_VACANT)
		non_tree_debug_internal(tree, &tree->nodes[node->first_child], indent + 1);

	if (node->next_sibling != NON_VACANT)
		non_tree_debug_internal(tree, &tree->nodes[node->next_sibling], indent);
}

void non_tree_debug(const non_tree* tree) {
	non_tree_debug_internal(tree, &tree->nodes[0], 0);
}

void non_print_error(FILE* file, const non_result result, const str source) {
	// TODO: Make good
	usize row;
	usize col;
	str_get_row_col(source, result.loc, &row, &col);
	fprintf(file, "%s at %lu:%lu\n", error_code_string(result.error), row, col);
}

static non_node_kind node_kind_from_token(enum token_type token_type) {
	switch (token_type) {
		case token_brace_left: case token_brace_right: return non_map;
		case token_bracket_left: case token_bracket_right: return non_list;
		case token_string: return non_string;
		case token_char: return non_char;
		case token_number: return non_number;
	}
	UNREACHABLE;
}

non_result non_parse(string s, non_tree* tree) {
	vec(token) tokens = {};

	const non_result tokenize_result = non_tokenize(s, &tokens);
	if (!tokenize_result.ok) {
		string_free(&s);
		return tokenize_result;
	}

	// Pre-compute sizes so we don't allocate more than we have to.
	u32 max_depth = 0;
	u32 current_depth = 0;
	u32 node_count = 1; // Account for root node.
	for (usize i = 0; i < tokens.len; i++) {
		const enum token_type type = tokens.data[i].type;
		/*if (type == token_string) {
			fwrite(tokens.data[i].str.data, tokens.data[i].str.len, 1, stdout);
			fputc('\n', stdout);
		} else {
			puts(token_type_string(type));
		}*/

		switch (type) {
			case token_brace_right: case token_bracket_right:
				current_depth--;
				break;
			case token_brace_left: case token_bracket_left:
				current_depth++;
				// Maps and lists will also be nodes.

			case token_string: case token_char: case token_number:
				node_count++;
				break;
		}

		if (current_depth > max_depth)
			max_depth = current_depth;
	}

	if (current_depth > 0) {
		string_free(&s);
		vec_free(&tokens);
		return non_error(non_unclosed_opener, s.len);
	}

	/*parse_ctx ctx = {
		.nodes = malloc(sizeof(non_node) * node_count),
		.node_count = node_count,
		.nodes_head = 1,
	};*/

	non_node* nodes = malloc(sizeof(non_node) * node_count);
	nodes[0] = (non_node){
		.kind = non_map,
		.next_sibling = NON_VACANT,
		.first_child = NON_VACANT,
		// .children_start = 1,
	};
	u32 node_last = 0;

	// Node scope stack to avoid using recursion! Don't mind the VLA...
	non_node* node_path[max_depth+1];
	// Contains indexes into the node array for the last child in each node scope.
	u32 last_child[max_depth+1];
	node_path[0] = &nodes[0];
	u32 path_last = 0;

	// False if we are parsing a key, true if we are parsing a value.
	// bool value_state = false;


	// Loop through the tokens, putting them into the tree.
	for (usize i = 0; i < tokens.len; i++) {
		// HACK:
		bool increment_path_last = false;

		const non_file_location loc = tokens.data[i].loc;
		switch (tokens.data[i].type) {
			case token_brace_left: case token_bracket_left:
				node_last++;
				// TODO: Separate these cases and remove node_kind_from_token?
				nodes[node_last] = (non_node){ .kind = node_kind_from_token(tokens.data[i].type) };
				// HACK:
				increment_path_last = true;
				/*path_last++;
				node_path[path_last] = &nodes[node_last];*/
				break;

			case token_brace_right: case token_bracket_right:
				if (node_path[path_last]->kind != node_kind_from_token(tokens.data[i].type)) {
					string_free(&s);
					vec_free(&tokens);
					free(nodes);
					return non_error(non_unexpected_closer, loc);
				}
				path_last--;
				// We don't want all the new-token logic after this switch in this case.
				goto next_token;

			case token_string:
				node_last++;
				nodes[node_last] = (non_node){ .kind = non_string, .str = tokens.data[i].str };
				break;
			case token_char:
				node_last++;
				nodes[node_last] = (non_node){ .kind = non_char, .chr = tokens.data[i].chr };
				break;
			case token_number:
				node_last++;
				nodes[node_last] = (non_node){ .kind = non_number, .num = tokens.data[i].num };
				break;
		}
		assert(path_last <= max_depth);
		assert(node_last < node_count);

		non_node* new_node = &nodes[node_last];
		non_node* parent_scope = node_path[path_last];
		non_node* parent_scope_last_added_child = &nodes[last_child[path_last]];

		// Default values.
		new_node->next_sibling = NON_VACANT;
		new_node->first_child = NON_VACANT;
		new_node->loc = loc;

		// Add the latest node as a first child of the parent if needed. Otherwise, add it as a sibling.
		if (parent_scope->first_child == NON_VACANT) {
			parent_scope->first_child = node_last;
		} else if (parent_scope->kind == non_map && parent_scope_last_added_child->first_child == NON_VACANT) {
			// If this is parented to a map, make key value pairs.
			parent_scope_last_added_child->first_child = node_last;
			parent_scope_last_added_child->child_count = 1;
			goto next_token;
		} else {
			parent_scope_last_added_child->next_sibling = node_last;
		}
		// Whatever happens, this is now the most recent child for the parent map/list.
		last_child[path_last] = node_last;
		// And as such, there the new node's parent has one more child now.
		parent_scope->child_count++;

		next_token:

		// HACK:
		if (increment_path_last) {
			path_last++;
			node_path[path_last] = &nodes[node_last];
		}
	}

	// const non_result parse_result = parse_scope(tokens.data, tokens.data + tokens.len, &ctx.nodes[0], &ctx);
	/*if (parse_result != non_ok) {
		string_free(&s);
		return parse_result;
	}*/

	*tree = (non_tree){
		.nodes = nodes,
		.node_count = node_count,
		.contents = s.data,
	};

	vec_free(&tokens);
	return non_ok;
}

void non_free(const non_tree tree) {
	free(tree.nodes);
	free(tree.contents);
}

/*void non_parse(FILE* file, non_node_visitor visitor, void* client_data, non_error_visitor error_visitor, void* error_visitor_client_data) {
	string buf = {0};
	vec_reserve(&buf, 20);

	// u32 col;
	// u32 row;



	string_free(&buf);
}*/

static non_result non_read_into_reflected_recursive(const non_tree* tree, const non_node* node, const type_info* type, void* data) {
	// Try direct conversions first.
	const auto conversions = trait_get(primitive_conversion_trait, type);
	if (conversions) switch (node->kind) {
		case non_string:
			if (conversions->from_string && conversions->from_string(data, node->str))
				return non_ok;
			break;
		case non_char:
			if (conversions->from_integer && conversions->from_integer(data, (s64)node->chr))
				return non_ok;
			break;
		case non_number:
			if (conversions->from_floating && conversions->from_floating(data, node->num))
				return non_ok;
			break;
		default:
	}

	// If that doesn't work, let's see if it's a list we can create.
	if (node->kind == non_list) {
		const auto list = trait_get(list_trait, type);
		const auto defaulter = trait_get(default_trait, type);
		if (list == nullptr)
			return non_error(non_target_type_not_list, node->loc);
		if (defaulter == nullptr)
			return non_error(non_target_type_not_defaulted, node->loc);

		defaulter->set_default(data);
		list->reserve(list, data, node->child_count);

		for (u32 child_idx = node->first_child; child_idx != NON_VACANT; child_idx = tree->nodes[child_idx].next_sibling) {
			const non_node* child = &tree->nodes[child_idx];

			void* element_data = list->push_new(list, data);
			non_read_into_reflected_recursive(tree, child, list->element_type, element_data);
		}
	}

	for (u32 child_idx = node->first_child; child_idx != NON_VACANT; child_idx = tree->nodes[child_idx].next_sibling) {
		const non_node* child = &tree->nodes[child_idx];

		switch (type->kind) {
			case type_info_struct:
				if (child->kind != non_string)
					return non_error(non_invalid_map_key, child->loc);

				for (usize field_idx = 0; field_idx < type->struct_data.field_count; field_idx++) {
					const type_info_field* field = &type->struct_data.fields[field_idx];
					if (!str_eq(field->name, child->str))
						continue;

					if (child->child_count != 1)
						return non_error(non_invalid_map_value, child->loc);

					void* field_ptr = type_info_resolve_field_ptr_allocate(field, data);
					if (field_ptr == nullptr)
						return non_error(non_unable_to_allocate_field, child->loc);

					non_try(non_read_into_reflected_recursive(tree, &tree->nodes[child->first_child], field->type, field_ptr));

					break;
				}
				break;
			case type_info_enum:
				if (child->kind == non_string) {
					for (usize i = 0; i < type->enum_data.variant_count; i++) {
						if (str_eq(type->enum_data.variants[i].name, child->str)) {
							nil_integer_to_bytes(type->enum_data.variants[i].value, data, type->size);
							break;
						}
					}
				} else if (child->kind == non_number) {
					if (!reflected_enum_contains_variant_value(type, (s64)child->num))
						return non_error(non_invalid_enum_data, child->loc);
					nil_integer_to_bytes((s64)child->num, data, type->size);
				} else {
					return non_error(non_invalid_enum_data, child->loc);
				}

				break;
			case type_info_union:
				panic("TODO: Implement unions");
			case type_info_opaque:
				return non_error(non_unable_to_read_opaque, child->loc);
			case type_info_kinds: UNREACHABLE;
		}
	}

	return non_ok;
}

non_result non_read_into_reflected(FILE* file, const type_info* type, void* data) {
	const string s = read_string_from_file(file);
	if (s.data == nullptr) {
		return non_error(non_failed_to_read_file, 0);
	}

	non_tree tree;
	const non_result parse_res = non_parse(s, &tree);
	if (!parse_res.ok) {
		// This is kinda dumb
		non_print_error(stderr, parse_res, string_as_slice(&s));
		return parse_res;
	}
	// non_tree_debug(&tree);

	const non_result read_res = non_read_into_reflected_recursive(&tree, &tree.nodes[0], type, data);
	if (!read_res.ok) {
		non_print_error(stderr, read_res, string_as_slice(&s));
		// TODO: memory can very easily leak from this, this is where an allocator would be very useful.
		return read_res;
	}

	non_free(tree);

	return non_ok;
}
