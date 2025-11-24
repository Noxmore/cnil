#include "reflect.h"

#include "ansi_colors.h"
#include "vec.h"
// #include "internal/cwisstable.h"

#include <stdio.h>
#include <stdlib.h>

/*static vec_anon(const type_info*) TYPE_REGISTRY;

void nil_register_type_info__(type_info* type) {
	type->index = TYPE_REGISTRY.len;
	vec_push(&TYPE_REGISTRY, type);
}
const type_info* type_registry_get(const usize index) {
	if (index >= TYPE_REGISTRY.len)
		return nullptr;

	return TYPE_REGISTRY.data[index];
}
usize type_registry_len() {
	return TYPE_REGISTRY.len;
}*/

// Because of anonymous struct, this has to be a separate type from trait_registry.

extern const type_info* __start_type_registry;
extern const type_info* __stop_type_registry;

/*const type_info* get_type_registry() {
	return __start_type_registry;
}*/
const type_info* type_registry_get(const usize index) {
	/*if (index >= type_registry_len())
		return nullptr;*/

	return *(&__start_type_registry + index);
}
usize type_registry_len() {
	return &__stop_type_registry - &__start_type_registry;
}

inline usize type_registry_index(const type_info* type) {
	return *type->index;
}

__attribute__((constructor))
static void set_type_registry_indexes() {
	// for (const type_info** it = &__start_type_registry; it < &__stop_type_registry; it++)
	// 	*(*it)->index
	for (usize i = 0; i < type_registry_len(); i++)
		*type_registry_get(i)->index = i;
}
/*usize type_registry_index(const type_info* type) {
	return type - &__start_type_registry;
}*/

/*CWISS_DECLARE_FLAT_HASHMAP(trait_registry_map, const type_info*, const void*);

typedef struct trait_registry {
	trait_registry_map map;
} trait_registry;*/

void free_reflected(const type_info* type, void* data) {
	const type_info_free_fn fn = get_reflected_free_fn(type);
	if (fn != nullptr) fn(data);
}

static type_info_free_fn* FREE_FUNCTIONS;

void type_info_register_free_fn(const type_info* type, type_info_free_fn fn) {
	if (FREE_FUNCTIONS == nullptr)
		FREE_FUNCTIONS = malloc(sizeof(*FREE_FUNCTIONS) * type_registry_len());

	FREE_FUNCTIONS[type_registry_index(type)] = fn;
}
type_info_free_fn get_reflected_free_fn(const type_info* type) {
	return FREE_FUNCTIONS[type_registry_index(type)];
}


const char* reflect_enum_name_from_variant_value(const type_info* type, const s64 variant_value) {
	if (type == nullptr) return nullptr;
	if (type->kind != type_info_enum) return nullptr;

	for (usize i = 0; i < type->enum_data.variant_count; i++) {
		if (variant_value == type->enum_data.variants[i].value)
			return type->enum_data.variants[i].name;
	}

	return nullptr;
}

static void indent(const u32 depth) {
	for (u32 i = 0; i < depth; i++)
		printf("  ");
}

static s64 get_any_sized_int(const void* data, const u8 type_size) {
	s64 value = 0;
	memcpy(&value, data, type_size);
	// Move the transferred bits to the right of the number.
	// UPDATE: Apparently it is already aligned correctly?
	// value >>= (s64)(sizeof(s64) - type_size);
	return value;
}

#define COLOR_COMMENT ANSI_STYLE(BLACK)
#define COLOR_KEYWORD ANSI_STYLE(RED)
#define COLOR_NUMBER ANSI_STYLE(MAGENTA)
#define COLOR_STRING ANSI_STYLE(GREEN)
#define COLOR_TYPE ANSI_STYLE(YELLOW)
#define COLOR_ENUM ANSI_STYLE(CYAN)
#define COLOR_ERROR ANSI_STYLE(RED)

static void debug_reflected_recursive(const void* obj, const type_info* type, const u32 depth) {
	if (type == nullptr) {
		printf(COLOR_ERROR "<null codec>");
		return;
	}

	switch (type->kind) {
		case type_info_struct:
			if (type->name != nullptr) {
				printf(COLOR_KEYWORD "struct " COLOR_TYPE "%s " ANSI_RESET "{\n", type->name);
			} else {
				puts(COLOR_KEYWORD "struct" ANSI_RESET " {");
			}

			for (usize i = 0; i < type->struct_data.field_count; i++) {
				indent(depth + 1);
				const type_info_field* field = &type->struct_data.fields[i];
				printf(".%s = ", field->name);

				const void* field_ptr = obj + field->struct_offset;

				if (field->is_pointer) {
					debug_reflected_recursive(*(void**)field_ptr, field->field_type, depth + 1);
				} else {
					debug_reflected_recursive(field_ptr, field->field_type, depth + 1);
				}

				puts(ANSI_RESET ",");
			}

			indent(depth);
			puts("}");

			break;
		case type_info_enum:
			const s64 value = get_any_sized_int(obj, type->size);
			const char* name = reflect_enum_name_from_variant_value(type, value);
			if (name != nullptr)
				printf(COLOR_ENUM "%s " COLOR_COMMENT "/* %li */", name, value);
			else
				printf(COLOR_ERROR "<invalid variant value: %li>", value);
			break;
		case type_info_opaque:
			if (type->opaque_data.kind == type_info_opaque_sint || type->opaque_data.kind == type_info_opaque_uint || type->opaque_data.kind == type_info_opaque_real)
				fputs(COLOR_NUMBER, stdout);
			else if (type->opaque_data.kind == type_info_opaque_string)
				fputs(COLOR_STRING "\"", stdout);

			type->opaque_data.to_string(stdout, obj);

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

#define REFLECT_INTEGER(T, FMT, KIND)                                          \
	static void T##_to_string(FILE* file, const void* ptr) {                    \
		fprintf(file, FMT, *(T*)ptr);                                            \
	}                                                                           \
	static void T##_from_string(const char* src, char** end_ptr, void* dst) {   \
		*(T*)dst = strtol(src, end_ptr, 10);                                     \
	}                                                                           \
	DEFINE_TYPE_INFO(T,                                                         \
		.kind = type_info_opaque,                                                \
		.mutable = true,                                                         \
		.name = #T,                                                              \
		.size = sizeof(T),                                                       \
		.align = alignof(T),                                                     \
		.annotations = nullptr,                                                  \
		.annotation_count = 0,                                                   \
		.free = nullptr,                                                         \
		.opaque_data = {                                                         \
			.kind = KIND,                                                         \
			.to_string = T##_to_string,                                           \
			.from_string = T##_from_string,                                       \
		},                                                                       \
	)

REFLECT_INTEGER(u8, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u16, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u32, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u64, "%lu", type_info_opaque_uint)
REFLECT_INTEGER(usize, "%lu", type_info_opaque_uint)

REFLECT_INTEGER(uint, "%u", type_info_opaque_uint)
REFLECT_INTEGER(ushort, "%u", type_info_opaque_uint)
REFLECT_INTEGER(ulong, "%lu", type_info_opaque_uint)

REFLECT_INTEGER(size_t, "%lu", type_info_opaque_uint)

REFLECT_INTEGER(s8, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s16, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s32, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s64, "%li", type_info_opaque_sint)

REFLECT_INTEGER(int, "%i", type_info_opaque_sint)
REFLECT_INTEGER(short, "%i", type_info_opaque_sint)
REFLECT_INTEGER(long, "%li", type_info_opaque_sint)

static void float_to_string(FILE* file, const void* ptr) {
	fprintf(file, "%f", *(float*)ptr);
}
static void float_from_string(const char* src, char** end_ptr, void* dst) {
	*(float*)dst = strtof(src, end_ptr);
}
DEFINE_TYPE_INFO(float,
	.kind = type_info_opaque,
	.mutable = true,
	.name = "float",
	.size = sizeof(float),
	.align = alignof(float),
	.annotations = nullptr,
	.annotation_count = 0,
	.free = nullptr,
	.opaque_data = {
		.kind = type_info_opaque_real,
		.to_string = float_to_string,
		.from_string = float_from_string,
	},
)

static void double_to_string(FILE* file, const void* ptr) {
	fprintf(file, "%f", *(double*)ptr);
}
static void double_from_string(const char* src, char** end_ptr, void* dst) {
	*(double*)dst = strtof(src, end_ptr);
}
DEFINE_TYPE_INFO(double,
	.kind = type_info_opaque,
	.mutable = true,
	.name = "double",
	.size = sizeof(double),
	.align = alignof(double),
	.annotations = nullptr,
	.annotation_count = 0,
	.free = nullptr,
	.opaque_data = {
		.kind = type_info_opaque_real,
		.to_string = double_to_string,
		.from_string = double_from_string,
	},
)