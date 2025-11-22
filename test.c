#define CNIL_TEST

#include "nil/panic.h"
#include "nil/slice.h"
#include "nil/string.h"
#include "nil/type_info.h"
#include "nil/vec_p.h"

int testing() {
	slice(s32) test = { .block_size = sizeof(s32), .len = 4, .data = (s32[]){2, 4, 7, 2} };
	test.data[0]++;
	return test.data[0];
}

#define structdef(T, ...) typedef struct T { __VA_ARGS__ } T;

struct deserializer {

};

struct serializer {
	void (*init)(void* state);

	void (*serialize_int)(void* state, FILE*, const char* name, s64 value);
	void (*serialize_float)(void* state, FILE*, const char* name, double value);
	void (*serialize_string)(void* state, FILE*, const char* name, str value);

	void (*start_object)(void* state, FILE*, const char* name);
	void (*end_object)(void* state, FILE*);

	void (*start_map)(void* state, FILE*, const char* name);
	void (*end_map)(void* state, FILE*);

	void (*free)(void* state);
};

/*const struct codec codec_field$codec = {
	.field_count = 2,
	.fields = (struct codec_field[]){
		{"name", codec_field_string},
		{"type", codec_field_enum},
	},
};
const struct codec codec$codec = {
	.field_count = 2,
	.fields = (struct codec_field[]){
		{"field_count", codec_field_int},
	},
};*/

enum entity_render_type {
	entity_render_type_solid,
	entity_render_type_transparent,
	entity_render_types,
};

// EXTERN_ENUM_CODEC(entity_render_type)

ENUM_CODEC(entity_render_type,
	entity_render_type_solid,
	entity_render_type_transparent,
)

typedef struct entity_render {
	enum entity_render_type type;
	str model_path;
	float scale;
	u16 index;
} entity_render;

DEFINE_CODEC(entity_render) {
	.type = codec_struct,
	.name = "entity_render",
	.type_size = sizeof(struct entity_render),
	.mutable = true,
	.struct_data = {
		.field_count = 4,
		.fields = (codec_field[]){
			{"type", codec_field_value, sizeof((struct entity_render){}.type), __builtin_offsetof(struct entity_render, type), &TYPE_CODEC_NAME(entity_render_type)},
			{"model_path", codec_field_value, sizeof(const char*), __builtin_offsetof(struct entity_render, model_path), &TYPE_CODEC_NAME(str)},
			{"scale", codec_field_value, sizeof(float), __builtin_offsetof(struct entity_render, scale), &TYPE_CODEC_NAME(float)},
			{"index", codec_field_value, sizeof(u16), __builtin_offsetof(struct entity_render, index), &TYPE_CODEC_NAME(u16)},
		},
	},
};

// typedef struct

/*STRUCT_CODEC(entity_render,
	(u16, index),
)*/

/*STRUCT_CODEC(entity_render,
	(entity_render_type, type),
	(const char*, model_path),
	(float, scale),
	(u16, index),
)*/

typedef usize entity_id;

typedef vec(struct pool_node$entity_render { struct pool_node$entity_render* prev; struct pool_node$entity_render* next; entity_render value }) pool$entity_render;

typedef struct entity_storage {
	pool$entity_render entity_renders;

	usize entities;
} entity_storage;


entity_id entity_spawn(entity_storage* storage) {
	struct pool$entity_render {  };
}

struct entity;

struct entity_type {
	const char* id;
	const codec* codec;

	void (*init)(struct entity*);
	void (*free)(struct entity*);

	// struct codec (*codec)(struct entity*);
};

struct entity {
	const struct entity_type* type;

	struct entity_render render;

	void* extra;
};

// #include <stdatomic.h>

void boopity(void** hello) {

}

int main() {
	// struct { int bap; } we = {2};
	// we = (auto){ 5 };

	struct entity_render foo = {
		.type = entity_render_type_transparent,
		.model_path = "This is a thing!",
		.scale = 3.32f,
		.index = 63,
	};


	debug_with_codec(&foo, &entity_render_$_codec);

	panic("Hello there!");

	vec(s32) list = nullptr;

	vec_push(&list, 5);
	vec_push(&list, 7);

	// s32 boop = list[0];
	// vec_pop(list, &boop);
	// vec_pop(list, &boop);

	vec_free(&list);

	/*string bap = string_new("This is a test string!");
	scope(string_free(&bap)) {
		puts(bap.data);
	}
	if (bap.data != nullptr) {
		puts("BAD");
	}*/
}

/*int main() {
	printf("%i\n", testing());
	printf("%i\n", testing());
	printf("%i\n", testing());
	// s32 my_ints[] = {2, 4, 7, 2};



	Foo foo = {
		.bap = 37,
		.bar = 6.36f,
		.baz = {0},
	};
	vecPush(&foo.baz, 77);
	vecPush(&foo.baz, 79);

	FILE* file = fopen("out.bin", "w");
	if (file == nullptr) {
		printf("Failed to open file\n");
		return 1;
	}

	// printf("Test");
	// Foo_binSerialize(file, &foo);
	binSerialize(Foo)(file, &foo);

	fclose(file);

	printf("Serialized\n");

	file = fopen("out.bin", "r");
	if (file == nullptr) {
		printf("Failed to open file for reading\n");
		return 1;
	}

	Foo foo2;
	// fread(&foo2, sizeof(Foo), 1, file);
	if (!Foo_binDeserialize(file, &foo2)) {
		printf("Failed to deserialize\n");
		return 1;
	}

	fclose(file);

	if (memcmp(&foo, &foo2, sizeof(Foo)) == 0) {
		printf("Sucess\n");
	} else {
		printf("Failed! Deserialized foo does not match\n");
	}
	vecDrop(&foo.baz);
	vecDrop(&foo2.baz);

	return 0;
}*/
