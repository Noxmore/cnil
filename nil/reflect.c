#include "reflect.h"

#include "ansi_colors.h"
#include "vec.h"
#include "internal/cwisstable.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "panic.h"

void nil_integer_to_bytes(s64 value, u8* out, usize n) {
	while (n--) {
		out[n] = (uint8_t)value;
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
		fprintf(file, COLOR_KEYWORD"%s"COLOR_ENUM" (primitive) " SIZE_ALIGN_FMT "\n", type->name, type->size, type->align);
		return;
	}

	fprintf(file, COLOR_KEYWORD"%s "COLOR_TYPE"%s " SIZE_ALIGN_FMT" {\n", short_type_kind(type), type->name, type->size, type->align);

	if (type->kind == type_info_struct || type->kind == type_info_union) {
		for (usize i = 0; i < type->struct_data.field_count; i++) {
			const type_info_field* field = &type->struct_data.fields[i];
			fprintf(file, "\t%s%s%s"ANSI_RESET"%s %s;\n", field->is_const ? COLOR_KEYWORD"const " : "",
				field->field_type->kind == type_info_opaque ? COLOR_KEYWORD : COLOR_TYPE,
				field->field_type->name,
				field->is_pointer ? "*" : "",
				field->name);
		}
	} else if (type->kind == type_info_enum) {
		for (usize i = 0; i < type->enum_data.variant_count; i++) {
			const type_info_variant* variant = &type->enum_data.variants[i];
			fprintf(file, "\t"COLOR_ENUM"%s"ANSI_RESET" = "COLOR_NUMBER"%li"ANSI_RESET",\n", variant->name, variant->value);
		}
	}

	fprintf(file, "}\n");
}
bool type_info_contains_annotation(const type_info* type, const char* annotation) {
	for (usize i = 0; i < type->annotation_count; i++) {
		if (strcmp(type->annotations[i], annotation) == 0)
			return true;
	}

	return false;
}

void free_reflected(const type_info* type, void* data) {
	if (type->free != nullptr) type->free(data);
}

CWISS_DECLARE_FLAT_HASHMAP(registry_index_map, const type_info*, usize);

typedef struct type_registry {
	vec_anon(const type_info*) linear;
	registry_index_map index_map;
} type_registry;

type_registry* type_registry_new() {
	return type_registry_new_with(40); // Semi-random number. Doesn't matter that much.
}

type_registry* type_registry_new_with(const usize reserve) {
	type_registry* reg = malloc(sizeof(type_registry));
	memset(reg, 0, sizeof(type_registry));

	vec_reserve(&reg->linear, reserve);
	reg->index_map = registry_index_map_new(reserve);

	return reg;
}

void type_register(type_registry* reg, const type_info* type) {
	if (type_registry_contains(reg, type))
		panic("Type registry already contains %s", type->name);

	registry_index_map_insert(&reg->index_map, &(registry_index_map_Entry){ .key = type, .val = reg->linear.len });
	vec_push(&reg->linear, type);
}
usize type_registry_size(const type_registry* reg) {
	return reg->linear.len;
}
const usize* type_registry_index(type_registry* reg, const type_info* type) {
	const auto iter = registry_index_map_find(&reg->index_map, &type);
	const auto entry = registry_index_map_Iter_get(&iter);
	return entry == nullptr ? nullptr : &entry->val;
}
bool type_registry_contains(const type_registry* reg, const type_info* type) {
	return registry_index_map_contains(&reg->index_map, &type);
}
const type_info* type_registry_get(const type_registry* reg, const usize index) {
	if (index >= reg->linear.len)
		return nullptr;
	return reg->linear.data[index];
}

void type_registry_free(type_registry* reg) {
	vec_free(&reg->linear);
	registry_index_map_destroy(&reg->index_map);
	free(reg);
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

/*s64 nil_get_any_sized_int(const void* data, const u8 type_size) {
	s64 value = 0;
	memcpy(&value, data, type_size);
	// Move the transferred bits to the right of the number.
	// UPDATE: Apparently it is already aligned correctly?
	// value >>= (s64)(sizeof(s64) - type_size);
	return value;
}*/

static void debug_reflected_recursive(const void* obj, const type_info* type, const u32 depth) {
	if (type == nullptr) {
		printf(COLOR_ERROR "<null codec>");
		return;
	}

	switch (type->kind) {
		case type_info_struct:
			if (type->name.data != nullptr) {
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
			const s64 value = nil_bytes_to_integer(obj, type->size);
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

			type->conversions.to_string(obj, stdout);

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
	static void T##_to_string(const void* self, FILE* file) {                   \
		fprintf(file, FMT, *(T*)self);                                           \
	}                                                                           \
	static double T##_to_floating(const void* self) {                           \
		return (double)*(T*)self;                                                \
	}                                                                           \
	static s64 T##_to_integer(const void* self) {                               \
		return (s64)*(T*)self;                                                   \
	}                                                                           \
	static bool T##_from_string(void* self, const str s) {                      \
		if (s.len >= 30) return false;                                           \
		char buf[30];                                                            \
		memcpy(buf, s.data, s.len);                                              \
		buf[s.len] = '\0';                                                       \
		char* end;                                                               \
		*(T*)self = strtol(buf, &end, 10);                                       \
		return end != buf;                                                       \
	}                                                                           \
	static bool T##_from_floating(void* self, const double v) {                 \
		*(T*)self = (T)v;                                                        \
		return true;                                                             \
	}                                                                           \
	static bool T##_from_integer(void* self, const s64 v) {                     \
		*(T*)self = (T)v;                                                        \
		return true;                                                             \
	}                                                                           \
	DEFINE_TYPE_INFO(T,                                                         \
		.kind = type_info_opaque,                                                \
		.size = sizeof(T),                                                       \
		.align = alignof(T),                                                     \
		.conversions = {                                                         \
			.to_string = T##_to_string,                                           \
			.to_floating = T##_to_floating,                                       \
			.to_integer = T##_to_integer,                                         \
			.from_string = T##_from_string,                                       \
			.from_floating = T##_from_floating,                                   \
			.from_integer = T##_from_integer,                                     \
		},                                                                       \
		.opaque_data.kind = KIND,                                                \
	)

REFLECT_INTEGER(u8, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u16, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u32, "%u", type_info_opaque_uint)
REFLECT_INTEGER(u64, "%lu", type_info_opaque_uint)
REFLECT_INTEGER(usize, "%lu", type_info_opaque_uint)
REFLECT_INTEGER(u128, "%w128u", type_info_opaque_uint) // TODO: Is this formatter implemented in Clang/GCC yet? If so, perhaps we should use it for all integers?

REFLECT_INTEGER(uint, "%u", type_info_opaque_uint)
REFLECT_INTEGER(ushort, "%u", type_info_opaque_uint)
REFLECT_INTEGER(ulong, "%lu", type_info_opaque_uint)

REFLECT_INTEGER(size_t, "%lu", type_info_opaque_uint)

REFLECT_INTEGER(s8, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s16, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s32, "%i", type_info_opaque_sint)
REFLECT_INTEGER(s64, "%li", type_info_opaque_sint)
REFLECT_INTEGER(s128, "%w128d", type_info_opaque_sint)

REFLECT_INTEGER(char, "%i", type_info_opaque_sint)
REFLECT_INTEGER(short, "%i", type_info_opaque_sint)
REFLECT_INTEGER(int, "%i", type_info_opaque_sint)
REFLECT_INTEGER(long, "%li", type_info_opaque_sint)

static void float_to_string(const void* self, FILE* file) {
	fprintf(file, "%f", *(float*)self);
}
static double float_to_floating(const void* self) {
	return (double)*(float*)self;
}
static s64 float_to_integer(const void* self) {
	return (s64)*(float*)self;
}
static bool float_from_string(void* self, const str s) {
	if (s.len >= 64) return false;
	char buf[64];
	memcpy(buf, s.data, s.len);
	buf[s.len] = '\0';

	char* end;
	*(float*)self = strtof(buf, &end);
	return end != buf;
}
static bool float_from_floating(void* self, const double v) {
	*(float*)self = (float)v;
	return true;
}
static bool float_from_integer(void* self, const s64 v) {
	*(float*)self = (float)v;
	return true;
}
DEFINE_TYPE_INFO(float,
	.kind = type_info_opaque,
	.size = sizeof(float),
	.align = alignof(float),
	.conversions = {
		.to_string = float_to_string,
		.to_floating = float_to_floating,
		.to_integer = float_to_integer,
		.from_string = float_from_string,
		.from_floating = float_from_floating,
		.from_integer = float_from_integer,
	},
	.opaque_data.kind = type_info_opaque_real,
)

static void double_to_string(const void* self, FILE* file) {
	fprintf(file, "%f", *(double*)self);
}
static double double_to_floating(const void* self) {
	return *(double*)self;
}
static s64 double_to_integer(const void* self) {
	return (s64)*(double*)self;
}
static bool double_from_string(void* self, const str s) {
	if (s.len >= 128) return false;
	char buf[128];
	memcpy(buf, s.data, s.len);
	buf[s.len] = '\0';

	char* end;
	*(float*)self = strtof(buf, &end);
	return end != buf;
}
static bool double_from_floating(void* self, const double v) {
	*(double*)self = v;
	return true;
}
static bool double_from_integer(void* self, const s64 v) {
	*(double*)self = (double)v;
	return true;
}
DEFINE_TYPE_INFO(double,
	.kind = type_info_opaque,
	.size = sizeof(double),
	.align = alignof(double),
	.conversions = {
		.to_string = double_to_string,
		.to_floating = double_to_floating,
		.to_integer = double_to_integer,
		.from_string = double_from_string,
		.from_floating = double_from_floating,
		.from_integer = double_from_integer,
	},
	.opaque_data.kind = type_info_opaque_real,
)

static bool string_from_string(void* self, str s) {
	*(string*)self = str_allocate(s);
	return true;
}
static void string_to_string(const void* self, FILE* file) {
	fputs(((string*)self)->data, file);
}
DEFINE_TYPE_INFO(string,
	.kind = type_info_opaque,
	.size = sizeof(string),
	.align = alignof(string),
	.free = (type_info_free_fn)string_free,
	.conversions = {
		.to_string = string_to_string,
		.from_string = string_from_string,
	},
	.opaque_data.kind = type_info_opaque_string,
)

static bool str_from_string(void* self, str s) {
	// TODO: This doesn't take ownership, should this function be nullptr?
	*(str*)self = s;
	return true;
}
static void str_to_string(const void* self, FILE* file) {
	fputs(((str*)self)->data, file);
}
DEFINE_TYPE_INFO(str,
	.kind = type_info_opaque,
	.size = sizeof(str),
	.align = alignof(str),
	.free = nullptr,
	.conversions = {
		.to_string = str_to_string,
		.from_string = str_from_string,
	},
	.opaque_data.kind = type_info_opaque_string,
)