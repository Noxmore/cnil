#include "reflect.h"

#include "trait.h"
#include "ansi_colors.h"
#include "vec.h"
#include "hashtable.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "panic.h"

void nil_integer_to_bytes(s64 value, u8* out, usize n) {
	while (n--) {
		out[n] = (u8)value;
		value >>= 8;
	}
}
s64 nil_bytes_to_integer(const u8* in, const usize n) {
	s64 value = 0;
	for (usize i = 0; i < n; i++) {
		value = (value << 8) | in[i];
	}
	return value;
}

/*usize type_ref_size(const type_ref type) {
	return type.pointer_layers > 0 ? sizeof(void*) : type.info->size;
}
usize type_ref_align(const type_ref type) {
	return type.pointer_layers > 0 ? alignof(void*) : type.info->align;
}*/

#define COLOR_COMMENT ANSI_STYLE(BRIGHT_BLACK)
#define COLOR_KEYWORD ANSI_STYLE(RED)
#define COLOR_NUMBER ANSI_STYLE(MAGENTA)
#define COLOR_STRING ANSI_STYLE(GREEN)
#define COLOR_TYPE ANSI_STYLE(YELLOW)
#define COLOR_ENUM ANSI_STYLE(CYAN)
#define COLOR_ERROR ANSI_STYLE(RED)

static const char* short_type_kind(const type_info* type) {
	switch (type->kind) {
		case type_info_struct: return "struct";
		case type_info_enum: return "enum";
		case type_info_union: return "union";
		default: return "";
	}
}

// #define SIZE_ALIGN_FMT "[ size: "COLOR_NUMBER"%lu"ANSI_RESET", align: "COLOR_NUMBER"%lu"ANSI_RESET" ]"
#define SIZE_ALIGN_FMT COLOR_COMMENT"/* size: %lu, align: %lu */"ANSI_RESET

void type_info_debug_print(const type_info* type, FILE* file) {
	if (type->kind == type_info_opaque) {
		fprintf(file, COLOR_KEYWORD"%s"COLOR_ENUM" (primitive) " SIZE_ALIGN_FMT "\n", type->name.data, type->size, type->align);
		return;
	}

	fprintf(file, COLOR_KEYWORD"%s "COLOR_TYPE"%s " SIZE_ALIGN_FMT" {\n", short_type_kind(type), type->name.data, type->size, type->align);

	if (type->kind == type_info_struct || type->kind == type_info_union) {
		for (usize field_idx = 0; field_idx < type->struct_data.field_count; field_idx++) {
			const type_info_field* field = &type->struct_data.fields[field_idx];

			fputc('\t', file);
			if (field->is_const)
				fputs("const ", file);

			fputs(field->type->kind == type_info_opaque ? COLOR_KEYWORD : COLOR_TYPE, file);
			fputs(field->type->name.data, file);
			fputs(ANSI_RESET, file);

			for (usize i = 0; i < field->pointer_layers; i++)
				fputc('*', file);

			fputc(' ', file);
			fputs(field->name.data, file);

			for (usize i = 0; i < field->const_array_layer_count; i++)
				fprintf(file, "[%lu]", field->const_array_layers[i]);

			fputs(";\n", file);
		}
	} else if (type->kind == type_info_enum) {
		for (usize i = 0; i < type->enum_data.variant_count; i++) {
			const type_info_variant* variant = &type->enum_data.variants[i];
			fprintf(file, "\t"COLOR_ENUM"%s"ANSI_RESET" = "COLOR_NUMBER"%li"ANSI_RESET",\n", variant->name.data, variant->value);
		}
	}

	fprintf(file, "}\n");
}
bool type_info_contains_annotation(const type_info* type, const str annotation) {
	for (usize i = 0; i < type->annotation_count; i++) {
		if (str_eq(type->annotations[i], annotation))
			return true;
	}

	return false;
}

void* type_info_resolve_field_ptr(const type_info_field* field, void* struct_data) {
	void* field_ptr = struct_data + field->struct_offset;

	for (usize i = 0; i < field->pointer_layers && field_ptr != nullptr; i++) {
		field_ptr = *(void**)field_ptr;
	}

	return field_ptr;
}

const void* type_info_resolve_field_const_ptr(const type_info_field* field, const void* struct_data) {
	// This doesn't change any data anyway, so we can just reuse the function.
	return type_info_resolve_field_ptr(field, (void*)struct_data);
}
void* type_info_resolve_field_ptr_allocate(const type_info_field* field, void* struct_data) {
	// TODO: Malloc annotations like the documentation says.
	return type_info_resolve_field_ptr(field, struct_data);
}

/*void reflected_object_free(reflected_object* obj) {
	free_reflected(obj->type, obj->data);
}*/

void free_reflected(const type_info* type, void* data) {
	if (data == nullptr)
		return;

	const destructor_trait* dtor = trait_get(destructor_trait, type);
	if (dtor) {
		dtor->free(data);
		return;
	}

	// TODO: union support
	if (type->kind != type_info_struct)
		return;

	for (usize i = 0; i < type->struct_data.field_count; i++) {
		const type_info_field* field = &type->struct_data.fields[i];
		free_reflected(field->type, type_info_resolve_field_ptr(field, data));

		// TODO: Boxed ptr freeing?
		assert(field->pointer_layers == 0);
	}
}

bool clone_reflected(const type_info* type, const void* from, void* into) {
	// We can't have partial clones because of possible allocations, so we have to make sure this object is cloneable first.
	if (from == nullptr || into == nullptr || !is_reflected_cloneable(type))
		return false;

	clone_reflected_unchecked(type, from, into);
	return true;
}

void clone_reflected_unchecked(const type_info* type, const void* from, void* into) {
	const clone_trait* cloner = trait_get(clone_trait, type);
	if (cloner) {
		cloner->clone_into(from, into);
		return;
	}

	if (type->kind != type_info_struct) {
		memcpy(into, from, type->size);
		return;
	}

	for (usize i = 0; i < type->struct_data.field_count; i++) {
		const type_info_field* field = &type->struct_data.fields[i];
		clone_reflected(field->type, from + field->struct_offset, into + field->struct_offset);
	}
}

bool is_reflected_cloneable(const type_info* type) {
	const clone_trait* cloner = trait_get(clone_trait, type);
	if (cloner) {
		return cloner->clone_into != nullptr;
	}

	// TODO: union support
	if (type->kind == type_info_union)
		return false;

	if (type->kind == type_info_struct) for (usize i = 0; i < type->struct_data.field_count; i++) {
		const type_info_field* field = &type->struct_data.fields[i];
		if (field->pointer_layers > 0)
			return false;

		if (!is_reflected_cloneable(field->type))
			return false;
	}

	return true;
}


typedef struct type_registry {
	vec_anon(const type_info*) linear;
	hashmap_anon(const type_info*, usize) index_map;
} type_registry;

type_registry* type_registry_new() {
	type_registry* reg = malloc(sizeof(type_registry));
	memset(reg, 0, sizeof(type_registry));

	vec_reserve(&reg->linear, staticalloc, 32); // Semi-random number. Doesn't matter that much.

	return reg;
}

void type_register(type_registry* reg, const type_info* type) {
	if (type_registry_contains(reg, type))
		panic("Type registry already contains %s", type->name.data);

	hashmap_insert(&reg->index_map, staticalloc, type, reg->linear.len);
	vec_push(&reg->linear, staticalloc, type);
}
usize type_registry_size(const type_registry* reg) {
	return reg->linear.len;
}
const usize* type_registry_index(type_registry* reg, const type_info* type) {
	/*const auto iter = registry_index_map_find(&reg->index_map, &type);
	const auto entry = registry_index_map_Iter_get(&iter);
	return entry == nullptr ? nullptr : &entry->val;*/
	return hashmap_get(&reg->index_map, &type);
}
bool type_registry_contains(const type_registry* reg, const type_info* type) {
	// return registry_index_map_contains(&reg->index_map, &type);
	return hashmap_contains(&reg->index_map, &type);
}
const type_info* type_registry_get(const type_registry* reg, const usize index) {
	if (index >= reg->linear.len)
		return nullptr;
	return reg->linear.data[index];
}

void type_registry_free(type_registry* reg) {
	vec_free(&reg->linear, staticalloc);
	hashmap_free(&reg->index_map, staticalloc);
	free(reg);
}

str reflect_enum_name_from_variant_value(const type_info* type, const s64 variant_value) {
	if (type == nullptr) return EMPTY_STR;
	if (type->kind != type_info_enum) return EMPTY_STR;

	for (usize i = 0; i < type->enum_data.variant_count; i++) {
		if (variant_value == type->enum_data.variants[i].value)
			return type->enum_data.variants[i].name;
	}

	return EMPTY_STR;
}
bool reflected_enum_contains_variant_value(const type_info* type, s64 variant_value) {
	if (type == nullptr || type->kind != type_info_enum) return false;

	for (usize i = 0; i < type->enum_data.variant_count; i++) {
		if (variant_value == type->enum_data.variants[i].value)
			return true;
	}

	return false;
}

static void indent(const u32 depth) {
	for (u32 i = 0; i < depth; i++)
		printf("  ");
}

static void debug_reflected_recursive(const void* obj, const type_info* type, const u32 depth) {
	if (type == nullptr) {
		printf(COLOR_ERROR "<null type>");
		return;
	}

	const auto list = trait_get(list_trait, type);
	if (list && list->const_iter) {
		printf("{\n");
		dynamic_iterator iter = list->const_iter(list, obj);
		void* element;

		while (iter.next(&iter, &element)) {
			indent(depth + 1);
			debug_reflected_recursive(element, list->element_type, depth + 1);
			puts(ANSI_RESET ",");
		}

		iter.free(&iter);

		indent(depth);
		putchar('}');
		return;
	}

	switch (type->kind) {
		case type_info_struct:
			if (type->name.data != nullptr) {
				printf(COLOR_KEYWORD "struct " COLOR_TYPE "%s " ANSI_RESET "{\n", type->name.data);
			} else {
				puts(COLOR_KEYWORD "struct" ANSI_RESET " {");
			}

			for (usize field_idx = 0; field_idx < type->struct_data.field_count; field_idx++) {
				indent(depth + 1);
				const type_info_field* field = &type->struct_data.fields[field_idx];
				printf(".%s = ", field->name.data);

				const void* field_ptr = type_info_resolve_field_const_ptr(field, obj);

				if (field_ptr)
					debug_reflected_recursive(field_ptr, field->type, depth + 1);
				else
					printf(COLOR_KEYWORD "nullptr");

				puts(ANSI_RESET ",");
			}

			indent(depth);
			puts("}");

			break;
		case type_info_enum:
			const s64 value = nil_bytes_to_integer(obj, type->size);
			const str name = reflect_enum_name_from_variant_value(type, value);
			if (name.data != nullptr)
				printf(COLOR_ENUM "%s " COLOR_COMMENT "/* %li */", name.data, value);
			else
				printf(COLOR_ERROR "<invalid variant value: %li>", value);
			break;
		case type_info_opaque:
			if (type->opaque_data.kind == type_info_opaque_sint || type->opaque_data.kind == type_info_opaque_uint || type->opaque_data.kind == type_info_opaque_real)
				fputs(COLOR_NUMBER, stdout);
			else if (type->opaque_data.kind == type_info_opaque_string)
				fputs(COLOR_STRING "\"", stdout);

			const primitive_conversion_trait* conversions = trait_get(primitive_conversion_trait, type);
			if (conversions && conversions->write_string)
				conversions->write_string(obj, stdout);
			else
				fputs("<No Writer>", stdout);

			if (type->opaque_data.kind == type_info_opaque_string)
				fputc('\"', stdout);
			break;
		default:
			printf(COLOR_ERROR "<invalid type: %u>", type->kind);
	}
}

void debug_reflected(const void* obj, const type_info* type) {
	debug_reflected_recursive(obj, type, 0);
	fflush(stdout);
}

DEFINE_TYPEDEF_INFO(void,
	.kind = type_info_opaque,
	.opaque_data.kind = type_info_opaque_void,
)

#define REFLECT_INTEGER(T, FMT, KIND)                                                     \
	static void T##_write_string(const void* self, FILE* file) {                           \
		fprintf(file, FMT, *(T*)self);                                                      \
	}                                                                                      \
	static string T##_to_string(const void* self, allocator_ref allocator) {               \
		return string_format(allocator, FMT, *(T*)self);                                    \
	}                                                                                      \
	static double T##_to_floating(const void* self) {                                      \
		return (double)*(T*)self;                                                           \
	}                                                                                      \
	static s64 T##_to_integer(const void* self) {                                          \
		return (s64)*(T*)self;                                                              \
	}                                                                                      \
	static bool T##_from_string(void* self, const str s, allocator_ref allocator) {        \
		if (s.len >= 30) return false;                                                      \
		char buf[30];                                                                       \
		memcpy(buf, s.data, s.len);                                                         \
		buf[s.len] = '\0';                                                                  \
		char* end;                                                                          \
		*(T*)self = strtol(buf, &end, 10);                                                  \
		return end != buf;                                                                  \
	}                                                                                      \
	static bool T##_from_floating(void* self, const double v, allocator_ref allocator) {   \
		*(T*)self = (T)v;                                                                   \
		return true;                                                                        \
	}                                                                                      \
	static bool T##_from_integer(void* self, const s64 v, allocator_ref allocator) {       \
		*(T*)self = (T)v;                                                                   \
		return true;                                                                        \
	}                                                                                      \
	DEFINE_TYPEDEF_INFO(T,                                                                 \
		.kind = type_info_opaque,                                                           \
		.opaque_data.kind = KIND,                                                           \
	)                                                                                      \
	IMPL_TRAIT(primitive_conversion_trait, T, {                                            \
		.write_string = T##_write_string,                                                   \
		.to_string = T##_to_string,                                                         \
		.to_floating = T##_to_floating,                                                     \
		.to_integer = T##_to_integer,                                                       \
		.from_string = T##_from_string,                                                     \
		.from_floating = T##_from_floating,                                                 \
		.from_integer = T##_from_integer,                                                   \
	})

REFLECT_INTEGER(u8, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u16, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u32, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u64, "%lu", type_info_opaque_uint)
REFLECT_INTEGER(usize, "%lu", type_info_opaque_uint)
REFLECT_INTEGER(u128, "%llu", type_info_opaque_uint)

REFLECT_INTEGER(uint, "%u", type_info_opaque_uint)
REFLECT_INTEGER(ushort, "%u", type_info_opaque_uint)
REFLECT_INTEGER(ulong, "%lu", type_info_opaque_uint)

REFLECT_INTEGER(size_t, "%lu", type_info_opaque_uint)

REFLECT_INTEGER(s8, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s16, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s32, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s64, "%li", type_info_opaque_sint)
REFLECT_INTEGER(s128, "%lli", type_info_opaque_sint)

REFLECT_INTEGER(char, "%i", type_info_opaque_sint)
REFLECT_INTEGER(short, "%i", type_info_opaque_sint)
REFLECT_INTEGER(int, "%i", type_info_opaque_sint)
REFLECT_INTEGER(long, "%li", type_info_opaque_sint)

static void float_write_string(const void* self, FILE* file) {
	fprintf(file, "%f", *(float*)self);
}
static string float_to_string(const void* self, allocator_ref allocator) {
	return string_format(allocator, "%f", *(float*)self);
}
static double float_to_floating(const void* self) {
	return (double)*(float*)self;
}
static s64 float_to_integer(const void* self) {
	return (s64)*(float*)self;
}
static bool float_from_string(void* self, const str s, allocator_ref allocator) {
	if (s.len >= 64) return false;
	char buf[64];
	memcpy(buf, s.data, s.len);
	buf[s.len] = '\0';

	char* end;
	*(float*)self = strtof(buf, &end);
	return end != buf;
}
static bool float_from_floating(void* self, const double v, allocator_ref allocator) {
	*(float*)self = (float)v;
	return true;
}
static bool float_from_integer(void* self, const s64 v, allocator_ref allocator) {
	*(float*)self = (float)v;
	return true;
}
DEFINE_TYPEDEF_INFO(float,
	.kind = type_info_opaque,
	.opaque_data.kind = type_info_opaque_real,
)
IMPL_TRAIT(primitive_conversion_trait, float, {
	.write_string = float_write_string,
	.to_string = float_to_string,
	.to_floating = float_to_floating,
	.to_integer = float_to_integer,
	.from_string = float_from_string,
	.from_floating = float_from_floating,
	.from_integer = float_from_integer,
})

static void double_write_string(const void* self, FILE* file) {
	fprintf(file, "%f", *(double*)self);
}
static string double_to_string(const void* self, allocator_ref allocator) {
	return string_format(allocator, "%f", *(double*)self);
}
static double double_to_floating(const void* self) {
	return *(double*)self;
}
static s64 double_to_integer(const void* self) {
	return (s64)*(double*)self;
}
static bool double_from_string(void* self, const str s, allocator_ref allocator) {
	if (s.len >= 128) return false;
	char buf[128];
	memcpy(buf, s.data, s.len);
	buf[s.len] = '\0';

	char* end;
	*(float*)self = strtof(buf, &end);
	return end != buf;
}
static bool double_from_floating(void* self, const double v, allocator_ref allocator) {
	*(double*)self = v;
	return true;
}
static bool double_from_integer(void* self, const s64 v, allocator_ref allocator) {
	*(double*)self = (double)v;
	return true;
}
DEFINE_TYPEDEF_INFO(double,
	.kind = type_info_opaque,
	.opaque_data.kind = type_info_opaque_real,
)
IMPL_TRAIT(primitive_conversion_trait, double, {
	.write_string = double_write_string,
	.to_string = double_to_string,
	.to_floating = double_to_floating,
	.to_integer = double_to_integer,
	.from_string = double_from_string,
	.from_floating = double_from_floating,
	.from_integer = double_from_integer,
})


static bool string_from_string(void* self, str s, allocator_ref allocator) {
	*(string*)self = str_allocate(s, allocator);
	return true;
}
static void string_write_string(const void* self, FILE* file) {
	fputs(((string*)self)->data, file);
}
static string string_to_string(const void* self, allocator_ref allocator) {
	return string_clone(self, allocator);
}
DEFINE_TYPEDEF_INFO(string,
	.kind = type_info_opaque,
	.opaque_data.kind = type_info_opaque_string,
)
// IMPL_FREE(string, string_free) // TODO: allocator-supplied free?
IMPL_TRAIT(primitive_conversion_trait, string, {
	.write_string = string_write_string,
	.to_string = string_to_string,
	.from_string = string_from_string,
})

static void str_write_string(const void* self, FILE* file) {
	fputs(((str*)self)->data, file);
}
static string str_to_string(const void* self, allocator_ref allocator) {
	return str_allocate(*(str*)self, allocator);
}
static bool str_from_string(void* self, str s, allocator_ref allocator) {
	/*// TODO: This doesn't take ownership, should this function be nullptr?
	*(str*)self = s;*/
	return false;
}
DEFINE_TYPEDEF_INFO(str,
	.kind = type_info_opaque,
	.opaque_data.kind = type_info_opaque_string,
)
IMPL_TRAIT(primitive_conversion_trait, str, {
	.write_string = str_write_string,
	.to_string = str_to_string,
	.from_string = str_from_string,
})